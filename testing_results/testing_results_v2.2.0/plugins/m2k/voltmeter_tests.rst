.. _m2k_voltmeter_tests:

Voltmeter - Test Suite
===============================================================================

.. note::

    User guide: :ref:`Scopy Overview <user_guide>`.


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
-------------------------------------------------------------------------------

.. _m2k-usb-voltmeter:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Channel 1 Operation
-------------------------------------------------------------------------------

**UID:** TST.M2K.VOLTMETER.CHANNEL_1_OPERATION

**RBP:** P0

**Description:** This test case verifies the functionality of the M2K voltmeter channel 1 operation.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-voltmeter>` setup.

**Steps:**
    1. Checking the DC Mode of channel 1
    2. Set channel 1 in DC Mode
            - **Expected Result:** The numerical value should indicate that it’s in VDC mode.
            - **Actual Result:**

..
  Actual test result goes here.
..

    3. Connect scope ch1+ to positive supply and ch1- to gnd
    4. Set the positive power supply voltage level to 3.3V
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 3.2V to 3.4V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    5. Connect scope ch1+ to negative supply and ch1- to gnd
    6. Set the negative power supply voltage level to -3.3V
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -3.2V to -3.4V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    7. Connect scope ch1+ to positive power supply and scope ch1- to negative supply
    8. Set the positive power supply voltage level to 5V and negative power supply to -5V
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 9.9V to 10.1V.
                - The history graph should follow.
            - **Actual Result:**

..
  Actual test result goes here.
..

    9. In step 3 replace scope ch1+ with scope ch1- and scope ch1- with scope ch1+ then repeat step 4
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -3.2V to -3.3V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    10. In step 5 replace scope ch1+ with scope ch1- and scope ch1- to scope ch1+ then repeat step 6
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around 3.2V to 3.3V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    11. In step 7 replace scope ch1+ with scope ch1- and scope ch1- with scope ch1+ then repeat step 8
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -9.9V to -10.1V .
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    12. Checking the AC Mode of channel 1 for low frequencies (20Hz to 800Hz)
    13. Set channel 1 in AC Mode (20Hz to 800Hz)
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    14. Connect scope ch1+ to AWG Ch1 and scope ch1- to gnd
    15. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Sine Wave
         - Amplitude: 2.828V
         - Offset: 0V
         - Frequency: 20Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    16. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Sine Wave
         - Amplitude: 5V
         - Offset: 0V
         - Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    17. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Square Wave,
         - Amplitude: 2.000V
         - Offset: 0
         - Frequency: 20Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    18. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Square Wave
         - Amplitude: 5V
         - Offset: 0V
         - Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    19. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Triangle Wave
         - Amplitude: 3.464V
         - Offset: 0V
         - Frequency: 20Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    20. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type:
         - Triangle Wave
         - Amplitude: 7V
         - Offset: 0V
         - Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    21. Checking the AC Mode of channel 1 for high frequencies (800Hz to 40kHz)
    22. Set channel 1 in AC Mode (800Hz to 40kHz)
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    23. Connect scope ch1+ to AWG Ch1 and scope ch1- to gnd
    24. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Sine Wave
         - Amplitude: 2.828V
         - Offset: 0V, Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    25. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Sine Wave
         - Amplitude: 5V
         - Offset: 0V, Frequency: 40kHz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    26. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Square Wave
         - Amplitude: 2.000V
         - Offset: 0V, Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    27. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Square Wave
         - Amplitude: 5V
         - Offset: 0V
         - Frequency: 40kHz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    28. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Triangle Wave
         - Amplitude: 3.464V
         - Offset: 0V
         - Frequency: 800Hz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    29. Set the Signal generator’s channel 1 configuration to the following setting
         - Waveform Type: Triangle Wave
         - Amplitude: 7V
         - Offset: 0V
         - Frequency: 40kHz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms.
                - The history graph should follow in 1s, 10s or 60s setting.
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


