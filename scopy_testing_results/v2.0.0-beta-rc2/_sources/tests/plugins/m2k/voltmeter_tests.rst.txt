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


Test 2: Channel 2 Operation
-------------------------------------------------------------------------------

**UID:** TST.M2K.VOLTMETER.CHANNEL_2_OPERATION

**Description:** This test case verifies the functionality of the M2K voltmeter channel 2 operation.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-voltmeter>` setup.

**Steps:**
    1. Checking the DC Mode of channel 2
    2. Set channel 2 in DC Mode
            - **Expected Result:** The numerical value should indicate that it’s in VDC mode.
            - **Actual Result:**

..
  Actual test result goes here.
..

    3. Connect scope ch2+ to positive supply and scope ch2- to gnd
    4. Set the positive power supply voltage level to 3.3V
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 3.2V to 3.
                - Tnd the history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    5. Connect scope ch2+ to negative supply and scope ch2- to gnd
    6. Set the negative power supply voltage level to -3.3V
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -3.2V to -3.4V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    7. Connect scope ch2+ to positive power supply and scope ch1- to negative supply
    8. Set the positive power supply voltage level to 5V and negative power supply to -5V
            - **Expected Result:**
                - The voltage displayed in the voltmeter should be around 9.9V to 10.
                - Tnd the history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    9. In step 3 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 4
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -3.2V to -3.3V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    10. In step 5 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 6
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around 3.2V to 3.3V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    11. In step 7 replace scope ch2+ with scope ch2- and and scope ch2- with scope ch2+ then repeat step 8
            - **Expected Result:**
                - The voltage displayed in voltmeter should be around -9.9V to -10.1V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    12. Checking the AC Mode of channel 2 for low frequencies (20Hz to 800Hz)
    13. Set channel 1 in AC Mode (20Hz to 800Hz)
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    14. Connect scope ch2+ to AWG ch1 and scope ch2- to gnd
    15. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Sine Wave
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

    16. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Sine Wave
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

    17. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Square Wave
         - Amplitude: 2.000V
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

    18. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Square Wave
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

    19. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Triangle Wave
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

    20. Set the Signal generator’s channel 1 configuration to the following setting:
         - Waveform Type: Triangle Wave
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

    21. Checking the AC Mode of channel 2 for high frequencies (800Hz to 40kHz)
    22. Set channel 1 in AC Mode (800Hz to 40kHz)
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    23. Connect scope ch2+ to AWG ch1 and scope ch2- to gnd
    24. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 2.828V, Offset: 0V, Frequency: 800Hz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    25. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Sine Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 1.66Vrms to 1.86Vrms and the history graph should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    26. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 2.000V, Offset: 0V, Frequency: 800Hz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    27. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Square Wave, Amplitude: 5V, Offset: 0V, Frequency: 40kHz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 2.4Vrms to 2.6Vrms and the history graph should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    28. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 3.464V, Offset: 0V, Frequency: 800Hz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 0.9Vrms to 1.1Vrms and the history graph should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    29. Set the Signal generator’s channel 1 configuration to the following setting Waveform Type: Triangle Wave, Amplitude: 7V, Offset: 0V, Frequency: 40kHz and Phase: 0
            - **Expected Result:** The voltage displayed in the voltmeter should be around 1.9Vrms to 2.1Vrms and the history graph should follow in 1s, 10s or 60s setting
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


Test 3: Channel 1 and Channel 2 Operation
-------------------------------------------------------------------------------

**UID:** TST.M2K.VOLTMETER.CHANNEL_1_AND_CHANNEL_2_OPERATION

**Description:** This test case verifies the functionality of the M2K voltmeter channel 1 and channel 2 operation.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-voltmeter>` setup.

**Steps:**
    1. Test both channels simultaneously in DC mode.
    2. Set channel 1 and 2 in DC Mode.
            - **Expected Result:** The numerical value should indicate that it’s in VDC mode.
            - **Actual Result:**

..
  Actual test result goes here.
..

    3. Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd.
    4. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V.
            - **Expected Result:**
                - The voltages shouldn’t interfere with each other.
                - Voltmeter’s channel 1 should be around 3.2V to 3.4V.
                - Voltmeter’s channel 2 should be around -4.6V to -4.4V.
                - The history graph should follow in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    5. Turn off the history graph of channel 1. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V.
            - **Expected Result:**
                - Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display.
                - Voltmeter’s channel 1 should be around 3.2V to 3.4V.
                - Voltmeter’s channel 2 should be around -4.6V to -4.4V. The history graph of channel 2 should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

    6. Turn off the history graph of channel 2. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V.
            - **Expected Result:**
                - Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display.
                - Voltmeter’s channel 1 should be around 3.2V to 3.4V.
                - Voltmeter’s channel 2 should be around -4.6V to -4.4V. The history graph of channel 1 should follow in 1s, 10s or 60s setting
            - **Actual Result:**

..
  Actual test result goes here.
..

   7. Turn off the history graph of both channels. Set the positive power supply voltage level to 3.3V and negative power supply to -4.5V.
           - **Expected Result:**
               - Turning off the history graph through the function shown on the picture shouldn’t reset or affect the voltage reading in the numerical display.
               - Voltmeter’s channel 1 should be around 3.2V to 3.4V.
               - Voltmeter’s channel 2 should be around -4.6V to -4.4V.
           - **Actual Result:**

..
  Actual test result goes here.
..

    8. Test both channels simultaneously in AC mode.
    9. Set channel 1 in low frequency AC mode and channel 2 in high frequency AC Mode.
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

       10. Connect scope ch1+ to AWG ch1 and scope ch1- to gnd. Connect scope ch2+ to AWG ch2 and scope ch2- to gnd.
       11. Set the following settings:
            - Signal generator’s channel 1
               - Waveform Type: Sine Wave
               - Amplitude: 2.828V
               - Offset: 0V
               - Frequency: 200Hz
               - Phase: 0
            - Signal generator’s channel 2
               - Waveform Type: Square Wave
               - Amplitude: 3
               - Offset: 0V
               - Frequency: 1kHz
               - Phase: 0
            - **Expected Result:**
               - Voltmeter’s channel 1 should be around 0.9Vrms to 1.1Vrms.
               - The voltage display for voltmeter’s channel 2 should be around 1.4Vrms to 1.6Vrms.
               - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    12. Set the following settings:
         - Signal generator’s channel 1
            - Waveform Type: Triangle Wave
            - Amplitude: 6.928V
            - Offset: 0V
            - Frequency: 200 Hz
            - Phase: 0.
         - Signal generator’s channel 2
            - Waveform Type: Sinewave
            - Amplitude: 2.828
            - Offset: 0V
            - Frequency: 1kH
            - Phase: 0
         - **Expected Result:**
            - Voltmeter’s channel 1 should be around 1.9Vrms to 2.1Vrms.
            - The voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.0Vrms.
            - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
         - **Actual Result:**

..
  Actual test result goes here.
..

   13. Test one channel in DC mode and other channel in AC mode simultaneously.
   14. Set channel 1 in DC Mode and channel 2 in AC Mode.
           - **Expected Result:**
               - The numerical value should indicate that channel 1 is in VDC mode.
               - Channel 2 is in AC mode, channel 2 should measure the Vrms.
           - **Actual Result:**

..
  Actual test result goes here.
..

    15. Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to AWG ch1 and scope ch2- to gnd.
    16. Set the positive power supply voltage level to 3.3V. Set the Signal generator’s channel 1 configuration to the following settings:
         - Waveform Type: Sine Wave
         - Amplitude: 2.828V
         - Offset: 0V
         - Frequency: 10kHz
         - Phase: 0
            - **Expected Result:**
                - Voltmeter’s channel 1 should be around 3.2V to 3.4V.
                - The voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    17. Set the positive power supply voltage level to 5V. Set the Signal generator’s channel 1 configuration to the following settings:
         - Waveform Type: Square Wave
         - Amplitude: 3V
         - Offset: 0V
         - Frequency: 10kHz
         - Phase: 0
            - **Expected Result:**
                - Voltmeter’s channel 1 should be around 4.9V to 5.1V.
                - The voltage display for voltmeter’s channel 2 should be around 1.4Vrms to 1.6Vrms.
                - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    18. Set channel 1 in AC Mode and channel 2 in DC Mode.
            - **Expected Result:** The numerical value should indicate that channel 1 is in AC mode and channel 2 is in DC mode, channel 1 should measure the Vrms.
            - **Actual Result:**

..
  Actual test result goes here.
..

    19. In step 15 replace scope ch1+ and scope ch1- with scope ch2+ and ch2- respectively and replace ch2+ and ch2- with ch1+ and ch1- respectively and repeat step 16.
            - **Expected Result:**
                - Voltmeter’s channel 2 should be around 3.2V to 3.4V.
                - Voltmeter’s channel 1 should be around 0.9Vrms to 1.1Vrms.
                - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
            - **Actual Result:**

..
  Actual test result goes here.
..

    20. In step 15 replace scope ch1+ and scope ch1- with scope ch2+ and ch2- respectively and replace ch2+ and ch2- with ch1+ and ch1- respectively and repeat step 17.
            - **Expected Result:**
                - Voltmeter’s channel 2 should be around 4.9V to 5.1V.
                - Voltmeter’s channel 1 should be around 1.4Vrms to 1.6Vrms.
                - The history graph should follow the voltage reading in 1s, 10s or 60s setting.
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


Test 4: Additional Features
-------------------------------------------------------------------------------

**UID:** TST.M2K.VOLTMETER.ADDITIONAL_FEATURES

**Description:** This test case verifies the functionality of the M2K voltmeter additional features.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-voltmeter>` setup.

**Steps:**
    1. Test Peak hold feature.
    2. Set channel 1 and 2 in DC Mode.
            - **Expected Result:** The numerical value should indicate that it’s in VDC mode.
            - **Actual Result:**

..
  Actual test result goes here.
..

    3. Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd.
    4. Turn on the Peak hold feature of the voltmeter.
            - **Expected Result:** The voltmeter window should now show the min and max indicator for both channels.
            - **Actual Result:**

..
  Actual test result goes here.
..

    5. Set +power supply to 2.5V and –power supply to -3V then turn on the power supply first before the voltmeter.
            - **Expected Result:**
                - The voltage displayed in channel 1’s max voltage should be around 2.4V to 2.6V and the min should still be 0V.
                - The voltage displayed on channel 2’s min voltage should be around -3.1V to -2.9V and the max voltage should be 0V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    6. Following step 5, Set +power supply to 5 V and –power supply to -5V.
            - **Expected Result:**
                - The voltage displayed in channel 1’s max voltage should be around 4.9V to 5.1V and the min should still be 0V.
                - The voltage displayed on channel 2’s min voltage should be around -5.1V to -4.9V and the max voltage should be 0V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    7. Connect scope ch1+ to negative supply and scope ch1- to gnd. Connect scope ch2+ to positive supply and scope ch2- to gnd
    8. Set +power supply to 2.5V and –power supply to -3V then turn on the power supply first before the voltmeter
            - **Expected Result:**
                - The voltage displayed in channel 2’s max voltage should be around 2.4V to 2.6V and the min should still be -5V.
                - The voltage displayed on channel 1’s min voltage should be around -3.1V to -2.9V and the max voltage should be 5V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    9. Following step 8, Set +power supply to 5 V and –power supply to -5V.
            - **Expected Result:**
                - The voltage displayed in channel 2’s max voltage should be around 4.9V to 5.1V and the min should still be -5V.
                - The voltage displayed on channel 1’s min voltage should be around -5.1V to -4.9V and the max voltage should be 5V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    10. Test the reset instrument feature.
    11. Stop Voltmeter instrument then click the reset instrument button for the peak hold features.
            - **Expected Result:** The max and min reading for both channels should return to 0V.
            - **Actual Result:**

..
  Actual test result goes here.
..

    12. Test Data logging feature.
    13. Set channel 1 in low frequency AC mode and channel 2 in high frequency AC Mode.
            - **Expected Result:** The numerical value should indicate that it’s in AC mode by showing that it’s reading the VRMS of the signal.
            - **Actual Result:**

..
  Actual test result goes here.
..

    14. Connect scope ch1+ to AWG ch1 and scope ch1- to gnd. Connect scope ch2+ to AWG ch2 and scope ch2- to gnd.
    15. Testing Append mode.
    16. Turn on the Data logging feature and choose Append.
    17. For the timer choose 5 seconds.
    18. Open a .csv file where the data will be logged.
            - **Expected Result:** The voltmeter reading should be recorded on the .csv file with 5 second interval.
            - **Actual Result:**

..
  Actual test result goes here.
..

    19. Set the  to the following settings:
         - **Signal generator’s channel 1 configuration:**
         - Waveform Type: Sine Wave
         - Amplitude: 2.828V
         - Offset: 0V
         - Frequency: 200Hz
         - Phase: 0.
         - **Signal generator’s channel 2 configuration:**
         - Waveform Type: Square Wave
         - Amplitude: 3
         - Offset: 0V
         - Frequency: 1kHz
         - Phase: 0
         - Run both the Signal generator and voltmeter.
            - **Expected Result:** Wait for about 1 minute to record at least 6 readings.
            - **Actual Result:**

..
  Actual test result goes here.
..

    20. Stop the voltmeter and open the .csv file using MS Excel.
            - **Expected Result:** The voltmeter reading should be recorded on the .csv file with 5 second interval.
            - **Actual Result:**

..
  Actual test result goes here.
..

    21. Change the timer for 20 seconds
            - **Expected Result:** The voltmeter reading should be recorded on the .csv file with 20 second interval.
            - **Actual Result:**

..
  Actual test result goes here.
..

    22. Set the  to the following settings:
         - **Signal generator’s channel 1 configuration:**
         - Waveform Type: Triangle Wave
         - Amplitude: 6.928V, Offset: 0V
         - Frequency: 200 Hz
         - Phase: 0
         - **Signal generator’s channel 2 configuration:**
         - Waveform Type: Sinewave
         - Amplitude: 2.828, Offset: 0V
         - Frequency: 1kHz
         - Phase: 0
            - **Expected Result:**
                - The voltage displayed in the voltmeter’s channel 1 should be around 1.9Vrms to 2.1Vrms.
                - The voltage display for voltmeter’s channel 2 should be around 0.9Vrms to 1.0Vrms.
                - Wait for about 1 minute to record at least 3 readings.
            - **Actual Result:**

..
  Actual test result goes here.
..

    23. Stop the voltmeter and open the .csv file using MS Excel.
            - **Expected Result:** The voltmeter reading should be recorded on the .csv file in continuation with the previous reading and should now record with 20 second interval.
            - **Actual Result:**

..
  Actual test result goes here.
..

    24. Testing overwrite mode
    25. Turn on the Data logging feature and choose Overwrite
            - **Expected Result:** Refer to the image for reference
            - **Actual Result:**

..
  Actual test result goes here.
..

    26. Repeat steps 17 to 23
            - **Expected Result:** The results should be the same but every run and stop of the voltmeter should replace the data on the .csv file chosen completely with the new readings.
            - **Actual Result:**

..
  Actual test result goes here.
..

    27. Test range feature
    28. Set channel 1 and 2 in DC Mode with range for both channels set to +-25V. Turn on the Peak hold feature of the voltmeter
            - **Expected Result:** The numerical value should indicate that it’s in VDC mode.
            - **Actual Result:**

..
  Actual test result goes here.
..

    29. Connect scope ch1+ to positive supply and scope ch1- to gnd. Connect scope ch2+ to negative supply and scope ch2- to gnd
    30. Set the positive power supply to 3.3V and the negative supply to -3.3V.
            - **Expected Result:** The voltmeter readings should be around [3.2V, 3.4V] for channel 1 and [-3.4V, -3.2V] for channel 2.
            - **Actual Result:**

..
  Actual test result goes here.
..

    31. Without disabling the power supply, change the range for both voltmeter channels to +-2.5V instead of +-25V.
            - **Expected Result:** “Out of range” should be raised for both channels.
            - **Actual Result:**

..
  Actual test result goes here.
..

    32. Still with range set to +-2.5V for both channels, set the power supply to output +100mV and -100mV.
            - **Expected Result:** The voltmeter readings should be around [0.097V, 0.103V] for channel 1 and [-0.103V, -0.097V] for channel 2.
            - **Actual Result:**

..
  Actual test result goes here.
..

    33. Without disabling the power supply, change the range for both voltmeter channels to +-25V instead of +-2.5V.
            - **Expected Result:** “Out of range” should be raised for both channels.
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

