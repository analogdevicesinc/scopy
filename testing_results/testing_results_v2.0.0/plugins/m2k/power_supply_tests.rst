.. _power_supply_tests:

Power Supply - Test Suite
=========================

.. note::

    User guide :ref:`Power Supply user guide<power_supply>`

The Power Supply tests are a set of tests that are run to verify the Power Supply 
instrument of the ADALM2000 plugin in Scopy.

The following apply for all the test cases in this suite.
If the test case has special requirements, they will be listed in the test case section.


.. note::
    .. list-table:: 
       :widths: 50 30 30 50 50
       :header-rows: 1

       * - Tester
         - Test Date
         - Scopy version
         - Plugin version (N/A if not applicable)
         - Comments
       * - 
         - 
         - 
         - 
         - 

Setup environment:
------------------

.. _adalm2000_ps_setup:

**Adalm2000.Device:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.
    - Connect **V+** to **1+** and **V-** to **2+** on the ADALM2000 device
      using loopback cables.

Test 1 - Check positive voltage output
--------------------------------------

.. _TST.PS.POSITIVE:

**UID:** TST.PS.POSITIVE

**Description:** This test verifies that the positive voltage output 
is working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_ps_setup>`
    - OS: ANY

**Steps:**
    1. Open the Power Supply instrument.
    2. Set Tracking ratio control to Independent Controls.
    3. Set the positive value to 3.3V and click enable.
    4. Monitor the power supply output with voltmeter.
        - **Expected Result**: The voltmeter reads values between 3.25V and 3.35V.
        - **Actual Result:**

..
  Actual test result goes here.
..

    5. Change the power supply output value to 1.8V.
        - **Expected Result**: The voltmeter reads values between 1.75V and 1.85V.
        - **Actual Result:**

..
  Actual test result goes here.
..

    6. Change the power supply output value to 5V.
        - **Expected Result**: The voltmeter reads values between 4.95V and 5.05V.
        - **Actual Result**:

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 2 - Check negative voltage output
--------------------------------------

.. _TST.PS.NEGATIVE:

**UID:** TST.PS.NEGATIVE

**Description:** This test verifies that the negative voltage output 
is working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_ps_setup>`
    - OS: ANY

**Steps:**
    1. Open the Power Supply instrument.
    2. Set Tracking ratio control to Independent Controls.
    3. Set the negative value to -3.3V and click enable.
    4. Monitor the power supply output with voltmeter.
        - **Expected Result**: The voltmeter reads values between -3.25V and -3.35V.
        - **Actual Result**:

..
  Actual test result goes here.
..

    5. Change the power supply output value to -1.8V.
        - **Expected Result**: The voltmeter reads values between -1.75V and -1.85V.
        - **Actual Result**:

..
  Actual test result goes here.
..

    6. Change the power supply output value to -5V.
        - **Expected Result**: The voltmeter reads values between -4.95V and -5.05V.
        - **Actual Result**:

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 3 - Check fine tuning
---------------------------

.. _TST.PS.FINE_TUNING:

**UID:** TST.PS.FINE_TUNING

**Description:** This test verifies that the fine tuning of the power supply 
is working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_ps_setup>`
    - OS: ANY

**Steps:**
    1. Open the Power Supply instrument.
    2. Set Tracking ratio control to Independent Controls.
    3. Set the knob to ±1V interval. No orange dot on the center.
    4. Set value to 3V. Then use +/- sign to change value with ±1V interval.
        - **Expected Result**: The value changes accordingly. Set Value ± 1V = the new value.
        - **Actual Result**:

..
  Actual test result goes here.
..

    5. Set the knob to ±100mV interval. Orange dot seen on the center.
    6. Set value to 300mV. Then use +/- sign to change value with ±100mV interval.
        - **Expected Result**: The value changes accordingly. Set Value ± 100mV = the new value.
        - **Actual Result**:

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).

        
Test 4 - Check Tracking mode
-----------------------------

.. _TST.PS.TRACKING:

**UID:** TST.PS.TRACKING

**Description:** This test verifies that the Tracking mode of the power supply 
is working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_ps_setup>`
    - OS: ANY

**Steps:**
    1. Open the Power Supply instrument.
    2. Set Tracking ratio control to Tracking.
    3. Set the positive output to 5V. Set tracking ratio to 50%.
        - **Expected Result**: The negative output value is automatically set to -2.5V.
        - **Actual Result**:

..
  Actual test result goes here.
..

    4. Set the tracking ratio to 100%:
        - **Expected Result**: The negative output value is automatically set to -5V.
        - **Actual Result**:

..
  Actual test result goes here.
..

    5. Set the tracking ratio to 0%:
        - **Expected Result**: The negative output value is automatically set to 0mV.
        - **Actual Result**:

..
  Actual test result goes here.
..

    6. Set Tracking ratio control to Independent.
    7. Set the positive output to 5V and verify the negative output:
        - **Expected Result**: The negative output value does not change.
        - **Actual Result**:

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).

