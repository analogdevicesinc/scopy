.. _oscilloscope_tests:

Oscilloscope - Test Suite
=========================

.. note::

   User guide: :ref:`Oscilloscope user guide<oscilloscope>`

The following test cases are designed to verify the functionality of 
the Oscilloscope plugin.


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

.. _adalm2000_osc_setup_ch1:

**Adalm2000.Osc.CH1:**
    - Open Scopy.
    - Connect the ADALM2000 device to the system.
    - Connect the ADALM2000 device in Scopy using the USB/network backend.
    - Open the Oscilloscope instrument and use only the CH1 settings and menus.

Depends on:
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with ADALM2000 plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.
    - Connect **W1** to **1+** and **GND** to **1-** on the ADALM2000 device
      using loopback cables.
    - Connect **W2** to **2+** and **GND** to **2-** on the ADALM2000 device
      using loopback cables.

Test 1 - Time base and Volts/div Knobs
--------------------------------------

.. _TST.OSC.TIME_VOLTS_1:

**UID:** TST.OSC.TIME_VOLTS_1

**RBP:** P0

**Description:** Check the increment/decrement operation of the time base and
volts/div knobs.

**Preconditions:**
    - :ref:`Adalm2000.Osc.CH1:<adalm2000_osc_setup_ch1>`
    - OS: ANY

**Steps:**
    1. Open the channel settings and set the knob to large increment 
       (NO orange dot in the center).
    2. Set the Time Base to 2us and the Volts/Div to 2 Volts.
    3. Use the - button 4 times and the + button 2 times on each knob:
        - **Expected result:**
            - The Time Base value changes to 1us, 500ns, 200ns, 100ns after the - button.
            - The Time Base value changes to 200ns and 500ns after the + button.
            - The Volts/Div value changes to 1V, 500, 200, 100 mV after the - button.
            - The Volts/Div value changes to 200mV and 500mV after the + button.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Set the knob to small increment (orange dot in the center).
       Set the Time Base to 2us and the Volts/Div to 2 Volts.
    5. Use the + and - button once on each knob:
        - **Expected result:**
            - The Time Base value changes to 3us and back to 2us.
            - The Volts/Div value changes to 3V and back to 2V.
        - **Actual result:**

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


