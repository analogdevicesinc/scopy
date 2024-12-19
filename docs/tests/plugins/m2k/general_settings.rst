General Settings - Test Case
===============================================

Initial Setup
-----------------------------------------------
In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open Scopy instrument. The interface should look like the pictures below:

Test Case
-----------------------------------------------

Setup:
    - M2K.*

Test 1: Installation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.GEN.SETUP.INSTALL

Description:
        - This test verifies the installation process of Scopy, ensuring that the software and its drivers are correctly installed, uninstalled, and reinstalled. It checks the sequence of prompts during installation and validates the correct setup of the ADALM2000 drivers.

OS:
        - any

Steps:
        * Step 1: If you have a different version of Scopy installed before, please uninstall along with the M2K drivers.
                * Expected Result: A dialog box will open asking if you want to uninstall Scopy and all of its contents. Upon clicking “Yes”, Scopy must be properly uninstalled.
        * Step 2: Download Scopy’s latest release on GitHub (https://github.com/analogdevicesinc/scopy/releases) and run. Follow the images of the promptings on the right for reference.
                * Expected Result: The prompts’ sequence will be the same as the ones posted here in the Step Resources.
        * Step 3: If you want to automatically install the ADALM2000 drivers, check the box indicating “Install drivers for ADALM2000” and click “Next.”
                * Expected Result: The prompt will look like the Step Resources picture.
        * Step 4: If you want to manually install the ADALM2000 drivers, uncheck the box indicating “Install drivers for ADALM2000” and click “Next.” Go to this link (https://github.com/analogdevicesinc/plutosdr-m2k-drivers-win/releases) to find your preferred version of ADALM2000 drivers.
                * Expected Result: The prompt will look like the Step Resources picture.
        * Step 5: Continue with the installation by following the promptings.
                * Expected Result: The prompts’ sequence will be the same as the ones posted here in the Step Resources.
        * Step 6: After Scopy finishes its installation, a different dialog box will open – the Device Driver Installation Wizard.
                * Expected Result: The prompts’ sequence will be the same as the ones posted here in the Step Resources. After completion, the “Device Driver Installation Wizard” dialog box will automatically close and direct you back to the Scopy installation setup.
        * Step 7: To use the application immediately, choose the “Yes, restart the computer now” option and click “Finish.”
                * Expected Result: The prompt will look like the Step Resources picture.
        * Step 8: If you opt to use the application for later, choose the “No, I will restart the computer later” option and click “Finish.”
                * Expected Result: The prompt will look like the Step Resources picture.

Test 2: M2K Connection and Calibration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.GEN.SETUP.M2KCONN

Description:
        - This test verifies the connection and calibration of the M2K board using both local and remote connections. It checks the auto and manual calibration features of the M2K board.

OS:
        - any

Steps:
    * Step 1: Connect the M2K board to the PC using a micro-USB connector.
            * Expected Result: The setup should look like the Steps Resources picture on the left.
    * Step 2: Click the M2K icon.
            * Expected Result: The instruments will appear on the left panel: Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, Logic Analyzer, Pattern Generator, Digital IO, Voltmeter, and Power Supply.
    * Step 3: Click “Identify.”
            * Expected Result: The “Ready” LED on the M2K board will rapidly blink 10 times indicating that it is the device identified by Scopy.
    * Step 4: Click “Connect.”
            * Expected Result: M2K will auto calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start. “Calibrate” button must be enabled.
    * Step 5: After connecting the M2K, manually calibrate the device by clicking the “Calibrate” button.
            * Expected Result: M2K will calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start.
    * Step 6: On the home menu, click the add “+” button and input the IP address of the desired M2K board to control. The default IP address is 192.168.2.1. Click add.
            * Expected Result: The setup should look like the Steps Resources picture on the left. The instruments will appear on the left panel: Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, Logic Analyzer, Pattern Generator, Digital IO, Voltmeter, and Power Supply.
    * Step 7: Click “Identify.”
            * Expected Result: The “Ready” LED on the M2K board will rapidly blink 10 times indicating that it is the device identified by Scopy.
    * Step 8: Click “Connect.”
            * Expected Result: M2K will auto calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start. “Calibrate” button must be enabled.
    * Step 9: Click “Forget device.”
            * Expected Result: The M2K board icon connected remotely will disappear on the panel.
    * Step 10: After connecting the M2K, manually calibrate the device by clicking the “Calibrate” button.
            * Expected Result: M2K will calibrate. The calibration indicator on Oscilloscope, Spectrum Analyzer, Network Analyzer, Signal Generator, and Voltmeter instrument should start.

Test 3: Save and Load Profile
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.GEN.SETUP.SAVELOAD

Description:
        - This test verifies the save and load profile feature of Scopy. It checks the ability to save and load profiles for each instrument and the ability to save and load the default profile.

OS:
        - any

Steps:
        * Step 1: Save the current profile. On the bottom part of the left panel, click the “Save” icon and save the profile as “default.ini.” Change any setting on any instrument and reload the profile.
                * Expected Result: The change made on a certain instrument should be undone and the default profile should load.
        * Step 2: Oscilloscope’s Settings: For Channel 1: Time Base: 100ns, Volts/Div: 2V, For Channel 2: Volts/Div: 5V. Signal Generator’s Settings: For Channel 1: Sinewave, 3Vpp, 3MHz, For Channel 2: Square Wave, 5Vpp, 900kHz. Spectrum Analyzer’s Settings: Frequency Sweep Setting: Start – 500kHz; Stop – 5MHz, Amplitude: Top – 10dBFS; Bottom – -140dBFS.
                * Expected Result: See Step Resource Picture for reference.
        * Step 3: Connect AWG channel 1 to Scope Ch1+ and AWG channel 2 to Scope Ch2+. Connect Scope Ch1- and Ch2- to GND.
                * Expected Result: See Step Resource Picture for reference.
        * Step 4: Run the Signal Generator instrument, and check the output on Oscilloscope and Spectrum Analyzer. Then save the profile as “profile1.ini.”
                * Expected Result: The output waveform on the Oscilloscope should be set in a way that the signals are can properly be seen compared with the default settings. In the Spectrum Analyzer the fundamental frequency of both signals should be present in the plot window set by the sweep setting.
        * Step 5: Reload the default setting by deleting the file in ``C:\Users\your_username\AppData\Roaming\ADI``. Reload “profile1.ini” and run the signal generator and oscilloscope or Spectrum Analyzer.
                * Expected Result: The result should be the same on the saved profile.
        * Step 6: Pattern Generator’s Settings: Enable DIO2, DIO3, DIO6, DIO7, DIO10, DIO11, DIO14 and DIO15. Group DIO1 and DIO2 set to random and 1MHz frequency. Group DIO10 and DIO11 and set to Binary Counter at 1MHz. Set the other enabled DIOs to clock at 1MHz. Logic Analyzer’s Settings: Group DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and DIO13 and set to parallel mode. Group DIO2 and DIO3 and set to parallel mode. Group DIO10 and DIO11 and set to parallel mode. Set the time base to 1us. Digital IO’s Settings: Set DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and DIO13 to output.
                * Expected Result: See step resource picture for reference.
        * Step 7: Run the three instrument and open Logic Analyzer instrument. Save the profile as “profile2.ini”.
                * Expected Result: See that the profile is saved on the desired location and the logic analyzer should be set in a way that the rising and falling edge of the signal is clearly seen.
        * Step 8: Reload the default setting by deleting the file in ``C:\Users\your_username\AppData\Roaming\ADI``. Reload “profile2.ini”, and run Pattern Generator, DigitalIO and Logic Analyzer.
                * Expected Result: The result should be the same on the saved profile.
        * Step 9: Power Supply’s Settings: Set to “Tracking Mode” with 35% tracking setting. Set positive supply to 3V and the negative supply should automatically be set to -1.05V. Signal Generator’s Settings: For Channel 1: Sinewave, 2Vpp, 10kHz, For Channel 2: Sinewave, 1Vpp, 10kHz, 90deg phase. Voltmeter Settings: For Channel 1: DC (Direct Current), History – OFF, For Channel 2: AC (20Hz – 40kHz), History – ON (1s). Network Analyzer: Reference: Channel 1, 2V Amplitude, Sweep: Linear, Start – 1kHz, Stop – 100kHz, Sample Count – 200, Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -100deg, Max. Phase – 100deg.
                * Expected Result: See step resource picture for reference.
        * Step 10: Connect Positive Supply to Scope Ch1+, connect AWG1 to Scope Ch2+, connect scope Ch1- and Scope Ch2- to GND.
                * Expected Result: See Step Resource Picture for reference.
        * Step 11: Run Power Supply, Voltmeter and Signal Generator to see if the voltmeter will be able to read 3V on channel 1 and 0.7V on channel 2. Save the profile as “profile3.ini”.
                * Expected Result: Channel 1’s history should be off and channel 2’s history should be present and the reading must be stable.
        * Step 12: Reload the default setting by deleting the file in ``C:\Users\your_username\AppData\Roaming\ADI``. Reload “profile3.ini”, and run Power Supply, Voltmeter and Signal Generator.
                * Expected Result: The result should be the same on the saved profile. The network analyzer’s setting should be retained.

Test 4: Preferences
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.GEN.SETUP.PREFERENCES

Description:
        - This test verifies the preferences feature of Scopy. It checks the seven sections of the preferences menu and the options available in each section.

OS:
        - any

Steps:
        * Step 1: Click the Preferences option located below the instrument options.
                * Expected Result: The Preferences menu should contain seven sections: General, Oscilloscope, Spectrum Analyzer, Logic Analyzer, Signal Generator, Network Analyzer, and Debug. Please see the step resource image for reference.
        * Step 2: Enable “Save session when closing Scopy.”
        * Step 3: Use Scopy and play with its instruments, changing the configurations and settings. Close Scopy and reopen.
        * Step 4: Enable “Show advanced device information.”
        * Step 5: On the Home menu, click the M2K icon and drag down to see the advanced device information.
                * Expected Result: A dialog box should appear confirming the reset command.
        * Step 6: Reset profile to default by deleting the files from ``C:\Users\your_username\AppData\Roaming\ADI``. Enable auto save feature. Load profile 1, profile 2 or profile 3 from Testing Save and Load feature steps. Close Scopy and Open.
        * Step 7: Following step 6, open Scopy and the current profile should be one of the profiles created from the Save and load test case. On the General Setting preference, the reset scopy is located in the lower right of the Scopy screen. Click reset scopy.
                * Expected Result: Reopening Scopy, the profile loaded should be the profile saved. Scopy should return to its default setting. Similar with deleting the files from folder.
        * Step 8: Under the Oscilloscope section, labels on the plot may be toggled on or off.
                * Expected Result: Checking the Oscilloscope plot, the labels must synchronize with the option chosen. See Step Resource image for reference.
        * Step 9: On the Spectrum Analyzer section, an option to search or not to search marker peaks in the visible domain is given.
                * Expected Result: See Step Resource Picture for reference.
        * Step 10: Signal Generator’s Settings: For Channel 1: Sinewave, 10Vpp, 500kHz. Spectrum Analyzer’s Sweep Settings: Start – 700kHz, Stop – 1MHz. Disable Channel 2. Connect AWG channel 1 to Scope Ch1+.
        * Step 11: Under the Marker Settings, click Marker 1 then “Peak.” Turn the Marker Table on and look for the marked frequencies.
                * Expected Result: A marker labeled M1 will automatically appear on the spectrum upon clicking Marker 1. Clicking “Peak” will put the Marker on the 500kHz mark.
        * Step 12: Under the Signal Generator section, The number of periods shown may be adjusted from 2 to 9.
                * Expected Result: The signal generator’s graphical representation must follow the desired number of periods on the lower frequency channel (if both channels are configured to output waveform signals). When numbers other than 2 to 9 are entered, the number and the line under it turns to red. See Step resource image for reference.
        * Step 13: On the Network Analyzer section, an option to display 0dB on the graph is available. Click to enable it.
                * Expected Result: See Step Resource Picture for reference.
        * Step 14: Construct a first-order low pass RC filter with the following components: R = 470 Ohms, C = 1uF. This will have a cut-off frequency of ~340 Hz.
                * Expected Result: The set up should look like in steps resources picture on the left.
        * Step 15: Network Analyzer’s Settings: Reference: Channel 1, 1V Amplitude, 0V Offset. Sweep: Logarithmic, Start – 10Hz, Stop – 500kHz, Sample Count - 100. Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -150deg, Max. Phase – 60deg. Run Network Analyzer.
                * Expected Result: The Bode Plot has 0dB on its labels. See Step Resource Picture for reference.
