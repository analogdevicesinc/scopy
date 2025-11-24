.. _m2k_general_settings_tests:

General Settings - Test Case
===============================================

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
-----------------------------------------------

.. _m2k-usb-general-settings:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.


Test 1: M2K Connection and Calibration
-----------------------------------------------

**UID:** TST.GEN.SETUP.M2KCONN

**RBP:** P3

**Description:** This test verifies the connection and calibration of the M2K board using both local and remote connections. It checks the auto and manual calibration features of the M2K board.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-general-settings>` setup.

**Steps:**
        1. Connect the M2K board to the PC using a micro-USB connector.
        2. Click the M2K icon.
                - **Expected Result:** The instruments will appear on the left panel: Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, Logic Analyzer, Pattern Generator, Digital IO, Voltmeter, and Power Supply.
                - **Actual Result**:

..
  Actual test result goes here.
..

        3. Click “Identify.”
                - **Expected Result:** The “Ready” LED on the M2K board will rapidly blink 10 times indicating that it is the device identified by Scopy.
                - **Actual Result**:

..
  Actual test result goes here.
..

        4. Click “Connect.”
                - **Expected Result:** M2K will auto calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start. “Calibrate” button must be enabled.
                - **Actual Result**:

..
  Actual test result goes here.
..

        5. After connecting the M2K, manually calibrate the device by clicking the “Calibrate” button.
                - **Expected Result:** M2K will calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start.
                - **Actual Result**:

..
  Actual test result goes here.
..

        6. On the home menu, click the add “+” button and input the IP address of the desired M2K board to control. The default IP address is 192.168.2.1. 
           In order for the connection via the ip:192.168.2.1 to take place, on macOS, the usb_ethernet_mode variable in m2k config.txt must be changed to
           ncm (for more details access `<https://wiki.analog.com/university/tools/pluto/users/customizing>`__). Click add.
                
                - **Expected Result:** The instruments will appear on the left panel: Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, Logic Analyzer, Pattern Generator, Digital IO, Voltmeter, and Power Supply.
                - **Actual Result**:

..
  Actual test result goes here.
..

        7. Click “Identify.”
                - **Expected Result:** The “Ready” LED on the M2K board will rapidly blink 10 times indicating that it is the device identified by Scopy.
                - **Actual Result**:

..
  Actual test result goes here.
..

        8. Click “Connect.”
                - **Expected Result:** M2K will auto calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start. “Calibrate” button must be enabled.
                - **Actual Result**:

..
  Actual test result goes here.
..

        9. Click “Forget device.”
                - **Expected Result:** The M2K board icon connected remotely will disappear on the panel.
                - **Actual Result**:

..
  Actual test result goes here.
..

        10. After connecting the M2K, manually calibrate the device by clicking the “Calibrate” button.
                 - **Expected Result:** M2K will calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start.
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


.. _m2k-test2-general-settings:

Test 2: Save and Load Profile
-----------------------------------------------

**UID:** TST.GEN.SETUP.SAVELOAD

**RBP:** P3

**Description:** This test verifies the save and load profile feature of Scopy. It checks the ability to save and load profiles for each instrument and the ability to save and load the default profile.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-general-settings>` setup.

**Steps:**
        1. Save the current profile. On the bottom part of the left panel, click the “Save” icon and save the profile as “default.ini.” Change any setting on any instrument and reload the profile.
                - **Expected Result:** The change made on a certain instrument should be undone and the default profile should load.
                - **Actual Result**:

..
  Actual test result goes here.
..

        2. Set the following settings:
                - **Oscilloscope’s Settings:**
                - Channel 1: Time Base: 100ns, Volts/Div: 2V
                - Channel 2: Volts/Div: 5V
                - **Signal Generator’s Settings:**
                - Channel 1: Sinewave, 3Vpp, 3MHz
                - Channel 2: Square Wave, 5Vpp, 900kHz
                - **Spectrum Analyzer’s Settings:**
                - Frequency Sweep Setting: Start – 500kHz; Stop – 5MHz, Amplitude: Top – 10dBFS; Bottom – -140dBFS
        3. Connect AWG channel 1 to Scope Ch1+ and AWG channel 2 to Scope Ch2+. Connect Scope Ch1- and Ch2- to GND.
        4. Run the Signal Generator instrument, and check the output on Oscilloscope and Spectrum Analyzer. Then save the profile as “profile1.ini.”
                - **Expected Result:** The output waveform on the Oscilloscope should be set in a way that the signals are can properly be seen compared with the default settings. In the Spectrum Analyzer the fundamental frequency of both signals should be present in the plot window set by the sweep setting.
                - **Actual Result**:

..
  Actual test result goes here.
..

        5. Reload the default setting by deleting the file in the settings folder. To open the setting folder, Open Preferences, make sure the "General" tab is selected, scroll to "Session" section and click the "Open" button. Reload “profile1.ini” and run the signal generator and oscilloscope or Spectrum Analyzer.
                - **Expected Result:** The result should be the same on the saved profile.
                - **Actual Result**:

..
  Actual test result goes here.
..

        6. Pattern Generator’s Settings:
                - Enable DIO2, DIO3, DIO6, DIO7, DIO10, DIO11, DIO14 and DIO15.
                - Group DIO2 and DIO3 set to random and 1MHz frequency.
                - Group DIO10 and DIO11 and set to Binary Counter at 1MHz.
                - Set the other enabled DIOs to clock at 1MHz.
                - Logic Analyzer’s Settings:
                - Group DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and DIO13 and set to parallel mode.
                - Group DIO2 and DIO3 and set to parallel mode. Group DIO10 and DIO11 and set to parallel mode.
                - Set the time base to 1us.
                - Digital IO’s Settings: Set DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and DIO13 to output.
        7. Run the three instrument and open Logic Analyzer instrument. Save the profile as “profile2.ini”.
                - **Expected Result:** See that the profile is saved on the desired location and the logic analyzer should be set in a way that the rising and falling edge of the signal is clearly seen.
                - **Actual Result**:

..
  Actual test result goes here.
..

        8. Reload the default setting by deleting the files in the setting folder (Click "Prefences" -> Scroll to "Session" -> Click "Open"). Reload “profile2.ini”, and run Pattern Generator, DigitalIO and Logic Analyzer.
                - **Expected Result:** The result should be the same on the saved profile.
                - **Actual Result**:

..
  Actual test result goes here.
..

        9. Power Supply’s Settings:
                - Set to “Tracking Mode” with 35% tracking setting.
                - Set positive supply to 3V and the negative supply should automatically be set to -1.05V.
                - Signal Generator’s Settings: For Channel 1: Sinewave, 2Vpp, 10kHz, For Channel 2: Sinewave, 1Vpp, 10kHz, 90deg phase.
                - Voltmeter Settings: For Channel 1: DC (Direct Current), History – OFF, For Channel 2: AC (20Hz – 40kHz), History – ON (1s).
                - Network Analyzer: Reference: Channel 1, 2V Amplitude, Sweep: Linear, Start – 1kHz, Stop – 100kHz, Sample Count – 200, Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -100deg, Max. Phase – 100deg.
        10. Connect Positive Supply to Scope Ch1+, connect AWG1 to Scope Ch2+, connect scope Ch1- and Scope Ch2- to GND.
        11. Run Power Supply, Voltmeter and Signal Generator to see if the voltmeter will be able to read 3V on channel 1 and 0.7V on channel 2. Save the profile as “profile3.ini”.
                 - **Expected Result:** Channel 1’s history should be off and channel 2’s history should be present and the reading must be stable.
                 - **Actual Result**:

..
  Actual test result goes here.
..

        12. Reload the default setting by deleting the files in the settings folder (Click "Prefences" -> Scroll to "Session" -> Click "Open"). Reload “profile3.ini”, and run Power Supply, Voltmeter and Signal Generator.
                - **Expected Result:** The result should be the same on the saved profile. The network analyzer’s setting should be retained.
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


Test 3: Preferences
-----------------------------------------------

**UID:** TST.GEN.SETUP.PREFERENCES

**RBP:** P3

**Description:** This test verifies the preferences feature of Scopy. It checks the sections of the preferences menu and the options available in each section.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-general-settings>` setup.

**Steps:**
        1. Click the Preferences option located below the instrument options. After the Preferences menu appears, click the "M2kPlugin" tab.
                - **Expected Result:** The Preferences menu should contain several sections: General, Analog tools and Logic tools.
                - **Actual Result:**

..
  Actual test result goes here.
..

        2. Enable “Save/Load Scopy session.” This setting is in the General tab.
        3. Use Scopy and play with its instruments, changing the configurations and settings. Close Scopy and reopen.
                - **Expected Result:** The settings are the ones configured by us.
                - **Actual Result:**

..
  Actual test result goes here.
..

        4. Reset profile to default by deleting the files the settings folder. Open Preferences, make sure the "General" tab is selected, scroll to "Session" section and click the "Open" button to view the settings file location. Close Scopy and reopen.
                - **Expected Result:** The settings configured by us are not longer available.
                - **Actual Result:**

..
  Actual test result goes here.
..
        
        5. Enable “Save/Load Scopy session”. Load profile 1, profile 2 or profile 3 from :ref:`Save and Load Profile <m2k-test2-general-settings>` test. Also, change the value of some "M2kPlugin" preferences. Close Scopy and reopen.
                - **Expected Result:** After reopening Scopy, the active profile will be the previously saved one, and the settings will remain as we set them.
                - **Actual Result:**
                        
..
  Actual test result goes here.
..

        6. Open the preferences menu and click on the "General" tab. Click on the "Reset" button from "Debug" section and then press "Restart".
                - **Expected Result:** Scopy should return to its default settings. Similar with deleting the files from folder.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. In the "M2kPlugin" tab, under the "Analog tools" section, labels on the plot may be toggled on or off.
                - **Expected Result:** Checking the Oscilloscope plot, the labels must synchronize with the option chosen.
                - **Actual Result:**

..
  Actual test result goes here.
..

        8. Enable "Only search marker peaks in visible domain" preference from "Analog tools" section.
        9. Signal Generator's Settings:
                - For Channel 1: Sinewave, 10Vpp, 500kHz.
                - Spectrum Analyzer's Sweep Settings: Start = 700kHz, Stop = 1MHz.
                - Disable Channel 2.
                - Connect AWG channel 1 to Scope Ch1+.
        10.  Under the Marker Settings, click Marker 1 then "Peak". Turn the Marker Table on and look for the marked frequencies.
                - **Expected Result:** A marker labeled M1 will automatically appear on the spectrum upon clicking Marker 1. Clicking "Peak" will put the Marker on the 500kHz mark.
                - **Actual Result:**

..
  Actual test result goes here.
..

        11.  Under the "Analog tools" section, the "Number of displayed periods" preference can be adjusted.
                - **Expected Result:** The signal generator's graphical representation must follow the 
                  desired number of periods on the lower frequency channel (if both channels are configured 
                  to output waveform signals). If the entered value is not a number greater than 0, the last 
                  valid value is restored.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12.  Enable "Always display 0db value on the graph" preference from "Analog tools" section.
        13.  Construct a first-order low pass RC filter with the following components: R = 470 Ohms, C = 1uF. This will have a cut-off frequency of ~340 Hz.
        14.  Network Analyzer's Settings
                - Reference: Channel 1
                - 1V Amplitude
                - 0V Offset
                - Sweep: Logarithmic
                - Start = 10Hz
                - Stop = 500kHz
                - Sample Count = 100
                - Display settings:
                - Min. Magnitude = -90dB
                - Max. Magnitude = 10dB
                - Min. Phase = -150deg
                - Max. Phase = 60deg
                - Run Network Analyzer.
                        - **Expected Result:** The Bode Plot has 0dB on its labels.
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

