.. _logic_analyzer_tests:

Logic Analyzer - Test Suite
===========================

.. note::

   User guide: :ref:`Logic Analyzer user guide<logic_analyzer>`

The Logic Analyzer tests are a set of tests that are run to verify the 
Logic Analyzer instrument of the ADALM2000 plugin in Scopy.

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

.. _adalm2000_device_setup:

**Adalm2000.Device:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.
    - Connect **W1** to **1+** and **GND** to **1-** using the pinout diagram. 

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - Channel Trigger Function
---------------------------------------------

.. _TST.LOGIC.CHN_TRIGGER:

**UID:** TST.LOGIC.CHN_TRIGGER

**RBP:** P0

**Description:** This test verifies the basic trigger functionality on individual 
digital channels - rising edge, falling edge, high, low, any edge.

**Preconditions:**
    - :ref:`Adalm2000.Device<adalm2000_device_setup>`
    - OS: ANY

**Prerequisites:**
    - :ref:`Pattern Generator<m2k_pattern_generator_tests>`
    - :ref:`Digital IO<digital_io_tests>`

**Steps:**
    1. Open the Logic Analyzer instrument.
    2. Enable DIO 0 and DIO 1 on the Logic Analyzer.
    3. Set the following parameters:
        - Sample rate to 50ksps.
        - Sample delay of -142 samples.
    4. Open the :ref:`Pattern Generator<pattern_generator>` instrument and enable DIO 1.
       Set the following parameters:
        
        - Pattern: clock.
        - Frequency: 100Hz.
        - Phase: 0 degrees.
        - Duty Cycle: 50%.
    5. Open the :ref:`Digital IO<digitalio>` instrument and set DIO 0 as output.
    6. In the Logic Analyzer set DIO0’s trigger to **rising edge** configuration.
    7. Run the Digital IO, Pattern Generator and Logic Analyzer instrument.
    8. In the Digital IO, change DIO0’s output from 0 to 1.
        - **Expected Result**: The logic analyzer initiates a capture.
        - **Actual Result:**

..
  Actual test result goes here.
..

    9. Stop the Logic Analyzer and set DIO0’s trigger to **rising edge** configuration.
    10. Run the Logic Analyzer instrument.
    11. In the Digital IO, change DIO0’s output from 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    12. Stop the Logic Analyzer and set DIO0’s trigger to **any edge** configuration.
    13. Run the Logic Analyzer instrument.
    14. In the Digital IO, change DIO0’s output from 0 to 1 or 1 to 0.
         - **Expected Result**: The logic analyzer initiates a capture.
         - **Actual Result:**

..
  Actual test result goes here.
..

    15. Stop the Logic Analyzer and set DIO0’s trigger to **low** configuration.
    16. Run the Logic Analyzer instrument.
    17. In the Digital IO, set DIO0’s output to 0.
         - **Expected Result**: The logic analyzer continuously captures the signal.
         - **Actual Result:**

..
  Actual test result goes here.
..

    18. Stop the Logic Analyzer and set DIO0’s trigger to **high** configuration.
    19. Run the Logic Analyzer instrument.
    20. In the Digital IO, set DIO0’s output to 1.
         - **Expected Result**: The logic analyzer continuously captures the signal.
         - **Actual Result:**

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


