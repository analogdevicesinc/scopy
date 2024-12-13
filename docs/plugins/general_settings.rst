General Settings - Test Case
============================

Initial Setup
-------------
In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool) from the following path on Windows: ``C:\Users\your_username\AppData\Roaming\ADI``.

Open Scopy instrument. The interface should look like the pictures below:

Test Case
---------
Installation
~~~~~~~~~~~~

+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| **Description**   | **Test Steps**                                          | **Steps Resources**         | **Expected Results**                                    |
+===================+=========================================================+=============================+=========================================================+
| Installation      | 1.1 If you have a different version of Scopy installed  |                             | A dialog box will open asking if you want to uninstall  |
|                   | before, please uninstall along with the M2K drivers.    |                             | Scopy and all of its contents. Upon clicking “Yes”,     |
|                   | Open Control Panel\All Control Panel Items\Programs     |                             | Scopy must be properly uninstalled.                     |
|                   | and Features, find Scopy, and click “Uninstall.”        |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                   | 1.2 Download Scopy’s latest release on GitHub           |                             | The prompts’ sequence will be the same as the ones      |
|                   | (https://github.com/analogdevicesinc/scopy/releases)    |                             | posted here in the Step Resources.                      |
|                   | and run. Follow the images of the promptings on the     |                             |                                                         |
|                   | right for reference:                                    |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Drivers for       | 1.3 If you want to automatically install the ADALM2000  |                             | The prompt will look like the Step Resources picture.   |
| ADALM2000         | drivers, check the box indicating “Install drivers for  |                             |                                                         |
|                   | ADALM2000” and click “Next.”                            |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                   | 1.4 If you want to manually install the ADALM2000       |                             | The prompt will look like the Step Resources picture.   |
|                   | drivers, uncheck the box indicating “Install drivers    |                             |                                                         |
|                   | for ADALM2000” and click “Next.” Go to this link        |                             |                                                         |
|                   | (https://github.com/analogdevicesinc/plutosdr-m2k-      |                             |                                                         |
|                   | drivers-win/releases) to find your preferred version    |                             |                                                         |
|                   | of ADALM2000 drivers.                                   |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Installation      | 1.5 Continue with the installation by following the     |                             | The prompts’ sequence will be the same as the ones      |
|                   | promptings.                                             |                             | posted here in the Step Resources.                      |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                   | 1.6 After Scopy finishes its installation, a different  |                             | The prompts’ sequence will be the same as the ones      |
|                   | dialog box will open – the Device Driver Installation   |                             | posted here in the Step Resources.                      |
|                   | Wizard.                                                 |                             |                                                         |
|                   |                                                         |                             | After completion, the “Device Driver Installation       |
|                   |                                                         |                             | Wizard” dialog box will automatically close and direct  |
|                   |                                                         |                             | you back to the Scopy installation setup.               |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Completing the    | 1.7 To use the application immediately, choose the      |                             | The prompt will look like the Step Resources picture.   |
| Scopy Setup       | “Yes, restart the computer now” option and click        |                             |                                                         |
| Wizard            | “Finish.”                                               |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                   | 1.8 If you opt to use the application for later, choose |                             | The prompt will look like the Step Resources picture.   |
|                   | the “No, I will restart the computer later” option and  |                             |                                                         |
|                   | click “Finish.”                                         |                             |                                                         |
+-------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+

M2K Connection and Calibration
==============================

   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | **Description**                                | **Test Steps**                                          | **Steps Resources**         | **Expected Results**                                    |
   +================================================+=========================================================+=============================+=========================================================+
   | Connect and Auto Calibration feature using     | 2.1 Connect the M2K board to the PC using a micro-USB   |                             | The set up should look like the Steps Resources picture |
   | local connection                               | connector.                                              |                             | on the left.                                            |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 2.2 Click the M2K icon.                                 |                             | The instruments will appear on the left panel:          |
   |                                                |                                                         |                             | Oscilloscope, Spectrum Analyzer, Network Analyzer,      |
   |                                                |                                                         |                             | Signal Generator, Logic Analyzer, Pattern Generator,    |
   |                                                |                                                         |                             | Digital IO, Voltmeter, and Power Supply.                |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 2.3 Click “Identify.”                                   |                             | The “Ready” LED on the M2K board will rapidly blink 10  |
   |                                                |                                                         |                             | times indicating that it is the device identified by    |
   |                                                |                                                         |                             | Scopy.                                                  |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 2.4 Click “Connect.”                                    |                             | M2K will auto calibrate. The calibration indicator on   |
   |                                                |                                                         |                             | Oscilloscope, Spectrum Analyzer, Network Analyzer,      |
   |                                                |                                                         |                             | Signal Generator and Voltmeter instrument should start. |
   |                                                |                                                         |                             | “Calibrate” button must be enabled.                     |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Manual Calibration feature using local         | 3.1 After connecting the M2K, manually calibrate the    |                             | M2K will calibrate. The calibration indicator on        |
   | connection                                     | device by clicking the “Calibrate” button.              |                             | Oscilloscope, Spectrum Analyzer, Network Analyzer,      |
   |                                                |                                                         |                             | Signal Generator and Voltmeter instrument should start. |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Connect and Auto Calibration feature using     | 4.1 On the home menu, click the add “+” button and      |                             | The set up should look like the Steps Resources picture |
   | remote connection                              | input the IP address of the desired M2K board to        |                             | on the left. The instruments will appear on the left    |
   |                                                | control. The default IP address is 192.168.2.1. Click   |                             | panel: Oscilloscope, Spectrum Analyzer, Network         |
   |                                                | add.                                                    |                             | Analyzer, Signal Generator, Logic Analyzer, Pattern     |
   |                                                |                                                         |                             | Generator, Digital IO, Voltmeter, and Power Supply.     |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 4.2 Click “Identify.”                                   |                             | The “Ready” LED on the M2K board will rapidly blink 10  |
   |                                                |                                                         |                             | times indicating that it is the device identified by    |
   |                                                |                                                         |                             | Scopy.                                                  |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 4.3 Click “Connect.”                                    |                             | M2K will auto calibrate. The calibration indicator on   |
   |                                                |                                                         |                             | Oscilloscope, Spectrum Analyzer, Network Analyzer,      |
   |                                                |                                                         |                             | Signal Generator and Voltmeter instrument should start. |
   |                                                |                                                         |                             | “Calibrate” button must be enabled.                     |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 4.4 Click “Forget device.”                              |                             | The M2K board icon connected remotely will disappear on |
   |                                                |                                                         |                             | the panel.                                              |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Manual Calibration feature using remote        | 5.1 After connecting the M2K, manually calibrate the    |                             | M2K will calibrate. The calibration indicator on        |
   | connection                                     | device by clicking the “Calibrate” button.              |                             | Oscilloscope, Spectrum Analyzer, Network Analyzer,      |
   |                                                |                                                         |                             | Signal Generator and Voltmeter instrument should start. |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+

Save and Load Profile
=====================

   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | **Description**                                | **Test Steps**                                          | **Steps Resources**         | **Expected Results**                                    |
   +================================================+=========================================================+=============================+=========================================================+
   | Save and Load Profile feature                  | 6.1 Save the current profile. On the bottom part of the |                             | The change made on a certain instrument should be       |
   |                                                | left panel, click the “Save” icon and save the profile  |                             | undone and the default profile should load.             |
   |                                                | as “default.ini.” Change any setting on any instrument  |                             |                                                         |
   |                                                | and reload the profile.                                 |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Save and Load Profile feature: Signal Generator| 6.2 Oscilloscope’s Settings:                            |                             | See Step Resource Picture for reference.                |
   | , Oscilloscope and Spectrum Analyzer           | a. For Channel 1: Time Base: 100ns, Volts/Div: 2V       |                             |                                                         |
   |                                                | b. For Channel 2: Volts/Div: 5V                         |                             |                                                         |
   |                                                | Signal Generator’s Settings:                            |                             |                                                         |
   |                                                | a. For Channel 1: Sinewave, 3Vpp, 3MHz                  |                             |                                                         |
   |                                                | b. For Channel 2: Square Wave, 5Vpp, 900kHz             |                             |                                                         |
   |                                                | Spectrum Analyzer’s Settings:                           |                             |                                                         |
   |                                                | a. Frequency Sweep Setting: Start – 500kHz; Stop – 5MHz |                             |                                                         |
   |                                                | b. Amplitude: Top – 10dBFS; Bottom – -140dBFS           |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.3 Connect AWG channel 1 to Scope Ch1+ and AWG channel |                             | See Step Resource Picture for reference.                |
   |                                                | 2 to Scope Ch2+. Connect Scope Ch1- and Ch2- to GND.    |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.4 Run the Signal Generator instrument, and check the  |                             | The output waveform on the Oscilloscope should be set   |
   |                                                | output on Oscilloscope and Spectrum Analyzer. Then save |                             | in a way that the signals are can properly be seen      |
   |                                                | the profile as “profile1.ini”                           |                             | compared with the default settings. In the Spectrum     |
   |                                                |                                                         |                             | Analyzer the fundamental frequency of both signals      |
   |                                                |                                                         |                             | should be present in the plot window set by the sweep   |
   |                                                |                                                         |                             | setting.                                                |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.5 Reload the default setting by deleting the file in  |                             | The result should be the same on the saved profile.     |
   |                                                | ``C:\Users\your_username\AppData\Roaming\ADI``. Reload  |                             |                                                         |
   |                                                | “profile1.ini” and run the signal generator and         |                             |                                                         |
   |                                                | oscilloscope or Spectrum Analyzer                       |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Save and Load Profile feature: Logic Analyzer, | 6.6 Pattern Generator’s Settings:                       |                             |                                                         |
   | Pattern Generator and Digital IO               | a. Enable DIO2, DIO3, DIO6, DIO7, DIO10, DIO11, DIO14   |                             |                                                         |
   |                                                | and DIO15.                                              |                             |                                                         |
   |                                                | b. Group DIO1 and DIO2 set to random and 1MHz frequency.|                             |                                                         |
   |                                                | c. Group DIO10 and DIO11 and set to Binary Counter at   |                             |                                                         |
   |                                                | 1MHz.                                                   |                             |                                                         |
   |                                                | d. Set the other enabled DIOs to clock at 1MHz.         |                             |                                                         |
   |                                                | Logic Analyzer’s Settings:                              |                             |                                                         |
   |                                                | a. Group DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and |                             |                                                         |
   |                                                | DIO13 and set to parallel mode.                         |                             |                                                         |
   |                                                | b. Group DIO2 and DIO3 and set to parallel mode.        |                             |                                                         |
   |                                                | c. Group DIO10 and DIO11 and set to parallel mode.      |                             |                                                         |
   |                                                | d. Set the time base to 1us.                            |                             |                                                         |
   |                                                | Digital IO’s Settings:                                  |                             |                                                         |
   |                                                | a. Set DIO0, DIO1, DIO4, DIO5, DIO8, DIO9, DIO12, and   |                             |                                                         |
   |                                                | DIO13 to output.                                        |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.7 Run the three instrument and open Logic Analyzer    |                             | See that the profile is saved on the desired location   |
   |                                                | instrument. Save the profile as “profile2.ini”.         |                             | and the logic analyzer should be set in a way that the  |
   |                                                |                                                         |                             | rising and falling edge of the signal is clearly seen.  |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.8 Reload the default setting by deleting the file in  |                             | The result should be the same on the saved profile.     |
   |                                                | ``C:\Users\your_username\AppData\Roaming\ADI``. Reload  |                             |                                                         |
   |                                                | “profile2.ini”, and run Pattern Generator, DigitalIO    |                             |                                                         |
   |                                                | and Logic Analyzer.                                     |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   | Save and Load Profile feature: Power Supply,   | 6.9 Power Supply’s Settings:                            |                             | See step resource picture for reference.                |
   | Voltmeter and Network Analyzer                 | a. Set to “Tracking Mode” with 35% tracking setting.    |                             |                                                         |
   |                                                | b. Set positive supply to 3V and the negative supply    |                             |                                                         |
   |                                                | should automatically be set to -1.05V.                  |                             |                                                         |
   |                                                | Signal Generator’s Settings:                            |                             |                                                         |
   |                                                | a. For Channel 1: Sinewave, 2Vpp, 10kHz                 |                             |                                                         |
   |                                                | b. For Channel 2: Sinewave, 1Vpp, 10kHz, 90deg phase    |                             |                                                         |
   |                                                | Voltmeter Settings:                                     |                             |                                                         |
   |                                                | a. For Channel 1: DC (Direct Current), History – OFF    |                             |                                                         |
   |                                                | b. For Channel 2: AC (20Hz – 40kHz), History – ON (1s)  |                             |                                                         |
   |                                                | Network Analyzer:                                       |                             |                                                         |
   |                                                | a. Reference: Channel 1, 2V Amplitude                   |                             |                                                         |
   |                                                | b. Sweep: Linear, Start – 1kHz, Stop – 100kHz, Sample   |                             |                                                         |
   |                                                | Count – 200                                             |                             |                                                         |
   |                                                | c. Display: Min. Magnitude – -90dB, Max. Magnitude –    |                             |                                                         |
   |                                                | 10dB, Min. Phase – -100deg, Max. Phase – 100deg         |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.10 Connect Positive Supply to Scope Ch1+, connect     |                             | See Step Resource Picture for reference.                |
   |                                                | AWG1 to Scope Ch2+, connect scope Ch1- and Scope Ch2-   |                             |                                                         |
   |                                                | to GND                                                  |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.11 Run Power Supply, Voltmeter and Signal Generator   |                             | Channel 1’s history should be off and channel 2’s       |
   |                                                | to see if the voltmeter will be able to read 3V on      |                             | history should be present and the reading must be       |
   |                                                | channel 1 and 0.7V on channel 2. Save the profile as    |                             | stable.                                                 |
   |                                                | “profile3.ini”.                                         |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
   |                                                | 6.12 Reload the default setting by deleting the file in |                             | The result should be the same on the saved profile. The |
   |                                                | ``C:\Users\your_username\AppData\Roaming\ADI``. Reload  |                             | network analyzer’s setting should be retained.          |
   |                                                | “profile3.ini”, and run Power Supply, Voltmeter and     |                             |                                                         |
   |                                                | Signal Generator.                                       |                             |                                                         |
   +------------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+

Preferences
===========

+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| **Description**                               | **Test Steps**                                          | **Steps Resources**         | **Expected Results**                                    |
+===============================================+=========================================================+=============================+=========================================================+
| Preferences Features                          | 7.1 Click the Preferences option located below the      | «change picture»            | The Preferences menu should contain seven sections:     |
|                                               | instrument options.                                     |                             | General, Oscilloscope, Spectrum Analyzer, Logic Analyzer|
|                                               |                                                         |                             | , Signal Generator, Network Analyzer, and Debug. Please |
|                                               |                                                         |                             | see the step resource image for reference.              |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| General                                       | 7.2 Enable “Save session when closing Scopy.”           | «put picture»               |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                                               | 7.3 Use Scopy and play with its instruments, changing   | «put picture»               |                                                         |
|                                               | the configurations and settings. Close Scopy and reopen.|                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                                               | 7.4 Enable “Show advanced device information.”          | «put picture»               |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
|                                               | 7.5 On the Home menu, click the M2K icon and drag down  | «put picture»               |                                                         |
|                                               | to see the advanced device information.                 |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Reset profile to default by deleting the      |                                                         |                             | Reopening Scopy, the profile loaded should be the       |
| files from                                    |                                                         |                             | profile saved.                                          |
| ``C:\Users\your_username\AppData\Roaming\ADI``|                                                         |                             |                                                         |
| . Enable auto save feature. Load profile 1,   |                                                         |                             |                                                         |
| profile 2 or profile 3 from Testing Save and  |                                                         |                             |                                                         |
| Load feature steps. Close Scopy and Open.     |                                                         |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Following step 7.7, open Scopy and the current|                                                         |                             | A dialog box should appear confirming the reset command.|
| profile should be one of the profiles created |                                                         |                             | Scopy should return to its default setting. Similar with|
| from the Save and load test case. On the      |                                                         |                             | deleting the files from  folder.                        |
| General Setting preference, the reset scopy is|                                                         |                             | ``C:\Users\your_username\AppData\Roaming\ADI``          |
| located in the lower right of the Scopy screen|                                                         |                             |                                                         |
| .Click reset scopy.                           |                                                         |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Oscilloscope                                  | Under the Oscilloscope section, labels on the plot may  |                             | Checking the Oscilloscope plot, the labels must         |
|                                               | be toggled on or off.                                   |                             | synchronize with the option chosen. See Step Resource   |
|                                               |                                                         |                             | image for reference.                                    |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Spectrum Analyzer                             | On the Spectrum Analyzer section, an option to search or|                             | See Step Resource Picture for reference.                |
|                                               |not to search marker peaks in the visible domain is given|                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Signal Generator’s Settings:                  |                                                         |                             | See Step Resource Picture for reference.                |
| a. For Channel 1: Sinewave, 10Vpp, 500kHz     |                                                         |                             |                                                         |
| Spectrum Analyzer’s Sweep Settings:           |                                                         |                             |                                                         |
| a. Start – 700kHz, Stop – 1MHz                |                                                         |                             |                                                         |
| b. Disable Channel 2.                         |                                                         |                             |                                                         |
| Connect AWG channel 1 to Scope Ch1+ .         |                                                         |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Under the Marker Settings, click Marker 1 then|                                                         |                             | A marker labeled M1 will automatically appear on the    |
| “Peak.” Turn the Marker Table on and look for |                                                         |                             | spectrum upon clicking Marker 1. Clicking “Peak” will   |
| the marked frequencies.                       |                                                         |                             | put the Marker on the 500kHz mark.                      |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Signal Generator                              | 7.7 Under the Signal Generator section, The number of   |                             | The signal generator’s graphical representation must    |
|                                               | periods shown may be adjusted from 2 to 9.              |                             | follow the desired number of periods on the lower       |
|                                               |                                                         |                             | frequency channel (if both channels are configured to   |
|                                               |                                                         |                             | output waveform signals). When numbers other than 2 to  |
|                                               |                                                         |                             | 9 are entered, the number and the line under it turns to|
|                                               |                                                         |                             | red. See Step resource image for reference.             |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Network Analyzer                              | On the Network Analyzer section, an option to display   |                             | See Step Resource Picture for reference.                |
|                                               | 0dB on the graph is available. Click to enable it.      |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Construct a first-order low pass RC filter    |                                                         |                             | The set up should look like in steps resources picture  |
| with the following components: R = 470 Ohms,  |                                                         |                             | on the left.                                            |
| C = 1uF. This will have a cut-off frequency of|                                                         |                             |                                                         |
| ~340 Hz.                                      |                                                         |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
| Network Analyzer’s Settings                   |                                                         |                             | The Bode Plot has 0dB on its labels. See Step Resource  |
| a. Reference: Channel 1, 1V Amplitude, 0V     |                                                         |                             | Picture for reference.                                  |
| Offset                                        |                                                         |                             |                                                         |
| b. Sweep: Logarithmic, Start – 10Hz, Stop –   |                                                         |                             |                                                         |
| 500kHz, Sample Count - 100                    |                                                         |                             |                                                         |
| c. Display: Min. Magnitude – -90dB, Max.      |                                                         |                             |                                                         |
| Magnitude – 10dB, Min. Phase – -150deg, Max.  |                                                         |                             |                                                         |
| Phase – 60deg                                 |                                                         |                             |                                                         |
| d. Run Network Analyzer                       |                                                         |                             |                                                         |
+-----------------------------------------------+---------------------------------------------------------+-----------------------------+---------------------------------------------------------+
