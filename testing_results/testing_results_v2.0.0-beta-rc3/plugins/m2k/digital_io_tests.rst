.. _digital_io_tests:

Digital IO - Test Suite
========================

.. note::

   User guide: :ref:`Digital IO user guide<digitalio>`

The Digital IO tests are a set of tests that are run to verify the 
Digital IO instrument of the ADALM2000 plugin in Scopy.

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
       * - Trifan Alexandra
         - 19/03/2025
         - v2.0.0-beta-rc3-8ade0b5
         - N/A
         - OS: Ubuntu 20.04 retesting

Setup environment:
------------------

.. _adalm2000_device_digital_setup:

**Adalm2000.Device:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.
    - Connect loopback cables using the pinout diagram between:
      - DIO 0-8
      - DIO 1-9
      - DIO 2-10
      - DIO 3-11
      - DIO 4-12
      - DIO 5-13
      - DIO 6-14
      - DIO 7-15

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - Check individual digital pin state
----------------------------------------------------

.. _TST.DIO.INDIVIDUAL:

**UID:** TST.DIO.INDIVIDUAL

**Description:** This test verifies that the individual digital pins are working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_digital_setup>`
    - OS: ANY

**Steps:**
    1. Open the DigitalIO instrument.
    2. Set DIO 0-7 and DIO 8-15 as individual pins using the combo box.
    3. Set DIO Channel 0 as **output**.
    4. Set DIO Channel 8 as **input**.
    5. Change the logic state of DIO 0 (0/1) multiple times and monitor DIO 8 state.
        - **Expected Result**:
            - When DIO 0 is set to logic 1, DIO 8 will be automatically set to logic 1
              (the corresponding led will be turned on).
            - When DIO 0 is set to logic 0, DIO 8 will be automatically set to logic 0
              (the corresponding led will be turned off).
        
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..
  
    6. Repeat steps 3 to 5 using DIO 8 as **output** (for step 3) and
       DIO 1 as **input** for (step 4).

        - **Expected Result**: All the expected results in step 5 are met.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 20.04

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 2 - Checking grouped digital pin states
----------------------------------------------------

.. _TST.DIO.GROUP:

**UID:** TST.DIO.GROUP

**Description:** This test verifies that the grouped digital pins DIO 0-7 and DIO 8-15
are working correctly.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_digital_setup>`
    - OS: ANY

**Steps:**
    1. Open the DigitalIO instrument.
    2. Set DIO 0-7 and DIO 8-15 as Group pins using the combo box.
    3. Set DIO 0-7 as **output** and DIO 8-15 as **input**.
    4. Set DIO 0-7 to value 0.
        - **Expected Result**: DIO 8-15 value indicates the same value as DIO 0-7 group.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    5. Set DIO 0-7 to value 128.
        - **Expected Result**: DIO 8-15 value indicates the same value as DIO 0-7 group.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    6. Set DIO 0-7 as **input** and DIO 8-15 as **output**.
    7. Set DIO 8-15 to value 0.
        - **Expected Result**: DIO 0-7 value indicates the same value as DIO 8-15 group.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    8. Set DIO 8-15 to value 128.
        - **Expected Result**: DIO 0-7 value indicates the same value as DIO 8-15 group.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 20.04

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 3 - Checking individual digital pin state voltmeter and power supply
--------------------------------------------------------------------------

.. _TST.DIO.INDIVIDUAL.VOLTMETER:

**UID:** TST.DIO.INDIVIDUAL.VOLTMETER

**Description:** This test verifies that the individual digital pins are working correctly using voltmeter and power supply. 

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_digital_setup>`
    - OS: ANY

**Steps:**

    1. Open the DigitalIO instrument.
    2. Set DIO 0-7 and DIO 8-15 as individual pins using the combo box.
    3. Set DIO Channel 0 as **output**.
    4. Set DIO Channel 8 as **input**.
    5. Connect DIO 0 to the analog pin **1+** and DIO 8 to the positive power 
       supply **V+** using the pinout diagram attached in resources.
    6. Set DIO 0 to logic 0 and open the :ref:`Voltmeter<voltmeter>` instrument.
       Start the instrument and monitor the voltage value on Channel 1.

        - **Expected Result**: The voltage displayed on the voltmeter is between -0.050V and 0.4V.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..
        
    7. Set DIO 0 to logic 1 and monitor the voltage value on Channel 1. Start the instrument.
        - **Expected Result**: The voltage displayed on the voltmeter is between 2.9V and 3.4V.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    8. Open the :ref:`Power Supply instrument<power_supply>` and set the positive 
       power supply to a voltage level between 0V and 0.8V.
       Monitor the DIO 8 logic state.

        - **Expected Result**: DIO 8 indicates logic 0 level.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    9. Open the :ref:`Power Supply instrument<power_supply>` and set the positive
       power supply to a voltage level between 2V and 3.3V.
       Monitor the DIO 8 logic state.

        - **Expected Result**: DIO 8 indicates logic 1 level.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

    10. Repeat steps 5 to 9 using DIO 8 as **output** (for step 3) and
        DIO 1 as **input** for (step 4).

        - **Expected Result**: All the expected results in step 6 to 11 are met.
        - **Actual Result**: As expected.

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 20.04

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).

