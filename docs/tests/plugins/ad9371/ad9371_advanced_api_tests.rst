.. _ad9371_advanced_api_tests:

AD9371 Advanced Tool API Test Suite
================================================================================

**Prerequisites:**
    - Scopy v2.2.0 or later with AD9371 plugin installed
    - AD9371 (or AD9375 for DPD/CLGC/VSWR tests) device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences

Test 1: CLK Settings
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_CLK_SETTINGS

**RBP:** P2

**Description:** Verify that CLK Settings debug attributes are readable and writable via the generic widget API.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read ``adi,clocks-device-clock_khz`` and ``adi,clocks-clk-pll-vco-freq_khz``
        - **Expected result:** Both return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read ``adi,clocks-clk-pll-vco-div`` combo widget
        - **Expected result:** Returns a raw numeric value (0, 1, 2, or 3)
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Write ``5`` to ``adi,clocks-clk-pll-hs-div``, readback, verify, restore
        - **Expected result:** Readback matches ``5``, original value is restored
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

Test 2: TX Settings
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_TX_SETTINGS

**RBP:** P2

**Description:** Verify that TX Settings debug attributes (config and profile) are readable and writable.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read ``adi,tx-settings-tx-channels-enable`` combo and verify non-empty
        - **Expected result:** Returns a raw numeric value
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify ``adi,tx-settings-tx-pll-use-external-lo`` checkbox returns ``0`` or ``1``
        - **Expected result:** Value is ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Write ``1000`` to ``adi,tx-settings-tx1-atten_mdb``, readback, verify, restore
        - **Expected result:** Readback matches ``1000``
        - **Actual result:**

..
  Actual test result goes here.
..
    4. Read TX profile attributes (``dac-div``, ``iq-rate_khz``, ``rf-bandwidth_hz``)
        - **Expected result:** All return non-empty values
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

Test 3: RX Settings
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_RX_SETTINGS

**RBP:** P2

**Description:** Verify that RX Settings debug attributes (config and profile) are readable.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read ``adi,rx-settings-rx-channels-enable`` combo and verify non-empty
        - **Expected result:** Returns a raw numeric value
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify checkboxes ``adi,rx-settings-rx-pll-use-external-lo`` and ``adi,rx-settings-real-if-data``
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read RX profile attributes (``adc-div``, ``iq-rate_khz``, ``rf-bandwidth_hz``)
        - **Expected result:** All return non-empty values
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

Test 4: DPD Settings (AD9375 only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_DPD_SETTINGS

**RBP:** P2

**Description:** Verify DPD debug configuration attributes. Skipped on AD9371 (non-AD9375).

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected (test skips on AD9371)

**Steps:**
    1. Read ``adi,dpd-model-version`` to detect AD9375; skip if unavailable
        - **Expected result:** Returns a value on AD9375, empty on AD9371
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read ``adi,dpd-num-weights`` and ``adi,dpd-samples``
        - **Expected result:** Both return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Verify checkboxes ``adi,dpd-high-power-model-update`` and ``adi,dpd-robust-modeling``
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    4. Write ``128`` to ``adi,dpd-damping``, readback, verify, restore
        - **Expected result:** Readback matches ``128``
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

**Result:** PASS/FAIL/SKIP

..
  The result of the test goes here (PASS/FAIL/SKIP).
..

Test 5: CLGC Settings (AD9375 only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_CLGC_SETTINGS

**RBP:** P2

**Description:** Verify CLGC debug configuration attributes. Skipped on AD9371 (non-AD9375).

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected (test skips on AD9371)

**Steps:**
    1. Read ``adi,clgc-tx1-desired-gain`` to detect availability; skip if empty
        - **Expected result:** Returns a value on AD9375
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read ``adi,clgc-tx1-control-ratio`` and ``adi,clgc-tx1-rel-threshold``
        - **Expected result:** Both return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Verify checkboxes ``adi,clgc-allow-tx1-atten-updates`` and ``adi,clgc-tx1-rel-threshold-en``
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    4. Write ``20000`` to ``adi,clgc-tx1-atten-limit``, readback, verify, restore
        - **Expected result:** Readback matches ``20000``
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

**Result:** PASS/FAIL/SKIP

..
  The result of the test goes here (PASS/FAIL/SKIP).
..

Test 6: VSWR Settings (AD9375 only)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_VSWR_SETTINGS

**RBP:** P2

**Description:** Verify VSWR debug configuration attributes. Skipped on AD9371 (non-AD9375).

**OS:** ANY

**Preconditions:**
    - AD9375 device is connected (test skips on AD9371)

**Steps:**
    1. Read ``adi,vswr-additional-delay-offset`` to detect availability; skip if empty
        - **Expected result:** Returns a value on AD9375
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read ``adi,vswr-tx1-vswr-switch-delay_us`` and ``adi,vswr-tx1-vswr-switch-gpio3p3-pin``
        - **Expected result:** Both return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Verify checkbox ``adi,vswr-tx1-vswr-switch-polarity``
        - **Expected result:** Returns ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    4. Write ``100`` to ``adi,vswr-path-delay-pn-seq-level``, readback, verify, restore
        - **Expected result:** Readback matches ``100``
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

**Result:** PASS/FAIL/SKIP

..
  The result of the test goes here (PASS/FAIL/SKIP).
..

Test 7: OBS Settings
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_OBS_SETTINGS

**RBP:** P2

**Description:** Verify Observation RX Settings debug attributes including OBS and sniffer profiles.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read OBS config combos (``obs-rx-lo-source``, ``default-obs-rx-channel``)
        - **Expected result:** Both return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify checkbox ``adi,obs-settings-real-if-data``
        - **Expected result:** Returns ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read OBS and sniffer profile attributes (``iq-rate_khz``, ``rf-bandwidth_hz``)
        - **Expected result:** All return non-empty numeric values
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

Test 8: Gain Setup
~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_GAIN_SETUP

**RBP:** P2

**Description:** Verify gain mode and gain index debug attributes for RX, ORX, and Sniffer paths.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read gain mode combos for RX, ORX, and Sniffer (``adi,rx-gain-mode``, etc.)
        - **Expected result:** All return non-empty values (0=MGC, 2=AGC, or 3=HYBRID)
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read gain indices and min/max values
        - **Expected result:** All return non-empty numeric values within [0, 255]
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Write ``200`` to ``adi,rx1-gain-index``, readback, verify, restore
        - **Expected result:** Readback matches ``200``
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

Test 9: AGC RX Peak Detector
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_AGC_RX_PEAK

**RBP:** P2

**Description:** Verify RX Analog Peak Detector AGC debug attributes (thresholds, counters, checkboxes).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read peak AGC thresholds (``apd-high-thresh``, ``apd-low-thresh``, ``hb2-high-thresh``)
        - **Expected result:** All return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify checkboxes ``apd-fast-attack`` and ``hb2-fast-attack``
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Write ``128`` to ``adi,rx-peak-agc-hb2-low-thresh``, readback, verify, restore
        - **Expected result:** Readback matches ``128``
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

Test 10: AGC RX Power Measurement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_AGC_RX_POWER

**RBP:** P2

**Description:** Verify RX Power Measurement AGC debug attributes (thresholds, duration, config combo).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read power AGC thresholds (upper high/low, lower high/low)
        - **Expected result:** All return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read measurement config combo and measurement duration
        - **Expected result:** Both return non-empty values
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

Test 11: AGC RX Config
~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_AGC_RX_CONFIG

**RBP:** P2

**Description:** Verify RX AGC Configuration debug attributes (gain indices, checkboxes, counters).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read AGC RX max/min gain indices
        - **Expected result:** Both return non-empty numeric values within [0, 255]
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify checkboxes (peak-threshold-mode, low-ths-prevent-gain-increase, reset-on-rx-enable)
        - **Expected result:** All return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read gain update counter and peak wait time
        - **Expected result:** Both return non-empty numeric values
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

Test 12: AGC OBS
~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_AGC_OBS

**RBP:** P2

**Description:** Verify Observation AGC debug attributes across peak, power, and config subsections.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read OBS peak AGC thresholds (``apd-high-thresh``, ``hb2-high-thresh``)
        - **Expected result:** Both return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read OBS power AGC threshold and measurement config
        - **Expected result:** Both return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read OBS AGC config max gain index and verify checkboxes
        - **Expected result:** Max gain index is non-empty; checkboxes return ``0`` or ``1``
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

Test 13: ARM GPIO
~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_ARM_GPIO

**RBP:** P2

**Description:** Verify ARM GPIO debug attributes (enable pins, pin modes, trigger/mode pins).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Verify checkboxes for enable pins and pin modes
        - **Expected result:** ``use-rx2-enable-pin``, ``use-tx2-enable-pin``, ``tx-rx-pin-mode``, ``orx-pin-mode`` all return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read ORX trigger and mode pins
        - **Expected result:** ``orx-trigger-pin``, ``orx-mode2-pin``, ``orx-mode1-pin``, ``orx-mode0-pin`` all return non-empty values
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

Test 14: GPIO
~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_GPIO

**RBP:** P2

**Description:** Verify GPIO source control combo debug attributes for 3.3V and low-voltage groups.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read GPIO 3.3V source control combos (``src-ctrl3_0``, ``src-ctrl7_4``, ``src-ctrl11_8``)
        - **Expected result:** All return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read GPIO low-voltage source control combos (``src-ctrl3_0`` through ``src-ctrl11_8``)
        - **Expected result:** All return non-empty values
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

Test 15: AUX DAC
~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_AUX_DAC

**RBP:** P2

**Description:** Verify AUX DAC debug attributes (per-DAC values, slopes, voltage references).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read DAC value and slope/vref combos for DAC 0 and DAC 1
        - **Expected result:** All return non-empty values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Write ``512`` to ``adi,aux-dac-value0``, readback, verify, restore
        - **Expected result:** Readback matches ``512``
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

Test 16: JESD Framer (RX + OBS)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_JESD_FRAMER

**RBP:** P2

**Description:** Verify JESD204 Framer debug attributes for both RX and OBS framers.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read RX framer attributes (``bank-id``, ``device-id``, ``m``, ``k``)
        - **Expected result:** All return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify RX framer checkboxes (``scramble``, ``external-sysref``)
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read OBS framer attributes (``bank-id``, ``device-id``) and verify ``scramble`` checkbox
        - **Expected result:** Values are non-empty; checkbox returns ``0`` or ``1``
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

Test 17: JESD Deframer
~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_JESD_DEFRAMER

**RBP:** P2

**Description:** Verify JESD204 Deframer debug attributes.

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Read deframer attributes (``bank-id``, ``device-id``, ``m``, ``k``)
        - **Expected result:** All return non-empty numeric values
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Verify deframer checkboxes (``scramble``, ``external-sysref``, ``new-sysref-on-relink``)
        - **Expected result:** All return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    3. Read ``eq-setting`` and ``lmfc-offset``
        - **Expected result:** Both return non-empty numeric values
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

Test 18: BIST
~~~~~~~~~~~~~~

**UID:** TST.AD9371.ADV_BIST

**RBP:** P2

**Description:** Verify BIST debug attributes (loopback checkboxes and PRBS combos).

**OS:** ANY

**Preconditions:**
    - AD9371 device is connected

**Steps:**
    1. Verify loopback checkboxes (``loopback_tx_rx``, ``loopback_tx_obs``)
        - **Expected result:** Both return ``0`` or ``1``
        - **Actual result:**

..
  Actual test result goes here.
..
    2. Read PRBS combos (``bist_prbs_rx``, ``bist_prbs_obs``)
        - **Expected result:** Both return non-empty values (0=Off, 1=PRBS7, 2=PRBS15, 3=PRBS31)
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
