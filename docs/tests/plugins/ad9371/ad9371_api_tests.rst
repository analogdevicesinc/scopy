.. _ad9371_api_tests:

AD9371 API Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.2.0 or later with AD9371 plugin installed
    - AD9371 (or AD9375 for DPD/CLGC/VSWR tests) device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: Plugin Loads
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.PLUGIN_LOADS

**RBP:** P0

**Description:** Verify that the AD9371 plugin loads and both tools are accessible.

**OS:** ANY

**Preconditions:**
    - Scopy is installed
    - AD9371 device is connected

**Steps:**
    1. Open Scopy application and connect to the AD9371 device
        - **Expected result:** Scopy launches without errors and connects to the device
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify "AD9371" and "AD9371 Advanced" tools appear in the tool list
        - **Expected result:** Both tools are visible and enabled in the plugin sidebar
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

Test 2: API Object Registration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.API_OBJECT_REGISTRATION

**RBP:** P1

**Description:** Verify the ad9371 API object is registered and accessible from JavaScript.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Connect to AD9371 device, open the JS console, run ``ad9371.getTools()``
        - **Expected result:** Returns a list containing "AD9371" and "AD9371 Advanced"
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

Test 3: Widget Keys Available
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.WIDGET_KEYS_AVAILABLE

**RBP:** P1

**Description:** Verify that getWidgetKeys() returns a non-empty list of registered widget keys from both tools.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getWidgetKeys()`` in the JS console
        - **Expected result:** Returns a list of widget keys including at least ``ad9371-phy/ensm_mode``, ``ad9371-phy/voltage0_in/hardwaregain``, ``ad9371-phy/altvoltage0_out/frequency``
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

Test 4: ENSM Mode Read-Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ENSM_MODE_READ_WRITE

**RBP:** P1

**Description:** Verify ENSM mode can be read and written via the API.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getEnsmMode()`` to read the current ENSM mode
        - **Expected result:** Returns a valid ENSM mode string (e.g., "radio_on", "wait", "alert")
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.setEnsmMode("wait")`` then ``ad9371.getEnsmMode()``
        - **Expected result:** Returns "wait" confirming the write was applied
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

Test 5: RX LO Frequency MHz Conversion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_LO_FREQUENCY_CONVERSION

**RBP:** P1

**Description:** Verify RX LO frequency getter returns MHz and setter accepts MHz with correct Hz conversion.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getRxLoFrequency()`` to read the current RX LO frequency
        - **Expected result:** Returns a value in MHz (e.g., "2400.000000"), not Hz
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.setRxLoFrequency("1000")`` then ``ad9371.getRxLoFrequency()``
        - **Expected result:** Returns "1000.000000" confirming the MHz-to-Hz conversion was applied correctly
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

Test 6: TX LO Frequency MHz Conversion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_LO_FREQUENCY_CONVERSION

**RBP:** P1

**Description:** Verify TX LO frequency getter returns MHz and setter accepts MHz with correct Hz conversion.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.setTxLoFrequency("2500")`` then ``ad9371.getTxLoFrequency()``
        - **Expected result:** Returns "2500.000000" confirming the MHz-to-Hz conversion was applied correctly
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

Test 7: RX Hardware Gain Per-Channel Read-Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_HARDWARE_GAIN_PER_CHANNEL

**RBP:** P1

**Description:** Verify per-channel RX hardware gain read and write for channels 0 and 1.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected (2Rx2Tx variant for channel 1 test)

**Steps:**
    1. Run ``ad9371.setRxHardwareGain(0, "15")`` then ``ad9371.getRxHardwareGain(0)``
        - **Expected result:** Returns "15" (or nearest valid step) for channel 0
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.setRxHardwareGain(1, "20")`` then ``ad9371.getRxHardwareGain(1)``
        - **Expected result:** Returns "20" (or nearest valid step) for channel 1
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

Test 8: TX Attenuation Negation Conversion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_ATTENUATION_NEGATION

**RBP:** P1

**Description:** Verify TX attenuation getter negates the raw hardwaregain value and setter negates before writing.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.setTxAttenuation(0, "10")`` then ``ad9371.getTxAttenuation(0)``
        - **Expected result:** Returns "10.00" (the API stores -10 in IIO and displays as positive attenuation)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.readWidget("ad9371-phy/voltage0_out/hardwaregain")`` to verify raw value
        - **Expected result:** Returns "-10" or "-10.00" (the negated raw IIO value)
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

Test 9: RX Gain Control Mode Combo
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_GAIN_CONTROL_MODE

**RBP:** P1

**Description:** Verify RX gain control mode can be read and written via the API.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getRxGainControlMode()``
        - **Expected result:** Returns a valid gain control mode string
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.setRxGainControlMode("manual")`` then ``ad9371.getRxGainControlMode()``
        - **Expected result:** Returns "manual"
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

Test 10: RX RSSI Read-Only
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_RSSI_READ_ONLY

**RBP:** P1

**Description:** Verify RX RSSI can be read and returns a valid value.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getRxRssi(0)``
        - **Expected result:** Returns a non-empty numeric string representing RSSI in dB
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

Test 11: Calibration Checkbox Read-Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.CALIBRATION_CHECKBOX_READ_WRITE

**RBP:** P1

**Description:** Verify calibration enable checkboxes can be toggled via the API.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getCalibrateRxQecEn()`` to read initial state
        - **Expected result:** Returns "0" or "1"
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.setCalibrateRxQecEn("1")`` then ``ad9371.getCalibrateRxQecEn()``
        - **Expected result:** Returns "1"
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Run ``ad9371.setCalibrateTxQecEn("1")`` then ``ad9371.getCalibrateTxQecEn()``
        - **Expected result:** Returns "1"
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

Test 12: Calibrate Trigger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.CALIBRATE_TRIGGER

**RBP:** P1

**Description:** Verify calibrate() triggers a calibration without error.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.calibrate()``
        - **Expected result:** No errors or exceptions in the JS console; calibration completes
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

Test 13: RX Quadrature Tracking Enable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_QUADRATURE_TRACKING

**RBP:** P1

**Description:** Verify per-channel RX quadrature tracking enable can be toggled.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.setRxQuadratureTracking(0, "1")`` then ``ad9371.getRxQuadratureTracking(0)``
        - **Expected result:** Returns "1"
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

Test 14: TX LO Leakage Tracking Enable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.TX_LO_LEAKAGE_TRACKING

**RBP:** P1

**Description:** Verify per-channel TX LO leakage tracking enable can be toggled.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.setTxLoLeakageTracking(0, "1")`` then ``ad9371.getTxLoLeakageTracking(0)``
        - **Expected result:** Returns "1"
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

Test 15: Observation RX Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.OBS_RX_ATTRIBUTES

**RBP:** P1

**Description:** Verify observation RX channel attributes can be read and written.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getObsRfBandwidth()``
        - **Expected result:** Returns a value in MHz (e.g., "100.000000")
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.getObsGainControlMode()``
        - **Expected result:** Returns a valid gain control mode string
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Run ``ad9371.setObsHardwareGain("25")`` then ``ad9371.getObsHardwareGain()``
        - **Expected result:** Returns "25" or nearest valid step
        - **Actual result:**

..
  Actual test result goes here.
..
    4. Run ``ad9371.getObsRfPortSelect()``
        - **Expected result:** Returns a valid port select string
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

Test 16: Sniffer LO Frequency MHz Conversion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.SNIFFER_LO_FREQUENCY_CONVERSION

**RBP:** P1

**Description:** Verify Sniffer LO frequency getter returns MHz and setter accepts MHz.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.setSnifferLoFrequency("3000")`` then ``ad9371.getSnifferLoFrequency()``
        - **Expected result:** Returns "3000.000000"
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

Test 17: DPD Tracking Enable (AD9375 Only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DPD_TRACKING_ENABLE

**RBP:** P2

**Description:** Verify DPD tracking enable can be toggled per TX channel (AD9375 only).

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected (DPD-capable variant)

**Steps:**
    1. Run ``ad9371.setDpdTrackingEn(0, "1")`` then ``ad9371.getDpdTrackingEn(0)``
        - **Expected result:** Returns "1"
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.getDpdTrackCount(0)``
        - **Expected result:** Returns a non-empty numeric string
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

Test 18: DPD Status String Map
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DPD_STATUS_STRING_MAP

**RBP:** P2

**Description:** Verify DPD status returns a human-readable string from the status index map.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.getDpdStatus(0)``
        - **Expected result:** Returns a human-readable string (e.g., "No Error" or an error description), not a raw integer
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

Test 19: DPD Model Error Scaling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DPD_MODEL_ERROR_SCALING

**RBP:** P2

**Description:** Verify DPD model error is scaled (raw ÷10) and returns a percentage string.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.getDpdModelError(0)``
        - **Expected result:** Returns a formatted string like "X.X %" (value divided by 10)
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

Test 20: DPD Reset Trigger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DPD_RESET_TRIGGER

**RBP:** P2

**Description:** Verify dpdReset() triggers a DPD reset without error.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.dpdReset(0)``
        - **Expected result:** No errors in the JS console
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

Test 21: CLGC Desired Gain Scaling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.CLGC_DESIRED_GAIN_SCALING

**RBP:** P2

**Description:** Verify CLGC desired gain getter divides by 100 and setter multiplies by 100.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.setClgcDesiredGain(0, "0.50")`` then ``ad9371.getClgcDesiredGain(0)``
        - **Expected result:** Returns "0.50" (confirming the ÷100/×100 conversion round-trips correctly)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.readWidget("ad9371-phy/voltage0_out/clgc_desired_gain")`` to verify raw value
        - **Expected result:** Returns "50" (0.50 × 100)
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

Test 22: CLGC Status and Read-Only Attributes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.CLGC_READ_ONLY_ATTRIBUTES

**RBP:** P2

**Description:** Verify CLGC read-only attributes return properly scaled values with units.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.getClgcStatus(0)``
        - **Expected result:** Returns a human-readable status string (not a raw integer)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.getClgcCurrentGain(0)``
        - **Expected result:** Returns a string ending with " dB" (value ÷100)
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Run ``ad9371.getClgcTxGain(0)``
        - **Expected result:** Returns a string ending with " dB" (value ÷20)
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

Test 23: VSWR Tracking and Status
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.VSWR_TRACKING_AND_STATUS

**RBP:** P2

**Description:** Verify VSWR tracking enable and status readback.

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected

**Steps:**
    1. Run ``ad9371.getVswrTrackingEn(0)``
        - **Expected result:** Returns "0" or "1"
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.getVswrStatus(0)``
        - **Expected result:** Returns a human-readable status string
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Run ``ad9371.getVswrForwardOrx(0)``
        - **Expected result:** Returns a string ending with " dBFS" (value ÷100 + 21)
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

Test 24: Generic Widget Read-Write
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.GENERIC_WIDGET_READ_WRITE

**RBP:** P1

**Description:** Verify the generic readWidget/writeWidget methods work for arbitrary widget keys.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.readWidget("ad9371-phy/ensm_mode")``
        - **Expected result:** Returns the same value as ``ad9371.getEnsmMode()``
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Run ``ad9371.writeWidget("ad9371-phy/ensm_mode", "wait")`` then ``ad9371.readWidget("ad9371-phy/ensm_mode")``
        - **Expected result:** Returns "wait"
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

Test 25: Refresh All Widgets
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.REFRESH_ALL_WIDGETS

**RBP:** P1

**Description:** Verify refresh() triggers a re-read of all registered widgets.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.refresh()``
        - **Expected result:** No errors; all widget values are refreshed from hardware (UI updates)
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

Test 26: Disconnect and Reconnect
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.DISCONNECT_RECONNECT

**RBP:** P1

**Description:** Verify API cleanup on disconnect and re-registration on reconnect.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Disconnect from the AD9371 device
        - **Expected result:** No crashes or errors during disconnect
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Reconnect to the AD9371 device, run ``ad9371.getTools()``
        - **Expected result:** Returns the tool list successfully; API is functional again
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

Test 27: RX Sampling Frequency MHz Conversion
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.RX_SAMPLING_FREQUENCY_CONVERSION

**RBP:** P1

**Description:** Verify RX sampling frequency read returns MSPS and write accepts MHz.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getRxSamplingFrequency()``
        - **Expected result:** Returns a value in MHz/MSPS (e.g., "122.880000"), not Hz
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

Test 28: Advanced Tool Widget Keys in Group
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADVANCED_WIDGET_KEYS_IN_GROUP

**RBP:** P2

**Description:** Verify that advanced tool section widgets (debug attributes) are registered in the shared widget group.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Run ``ad9371.getWidgetKeys()`` and check for an advanced-tool debug attribute key
        - **Expected result:** The key list includes debug attribute keys from the advanced tool (e.g., keys containing "adi,clocks-device-clock_khz" or similar debug attributes)
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
