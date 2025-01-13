M2K Spectrum Analyzer - Test Suite
====================================================================================================

Initial Setup
----------------------------------------------------------------------------------------------------
In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool).

Test Case
----------------------------------------------------------------------------------------------------

Setup:
        - M2K.*

Test 1: Channel 1 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.SA.CHANNEL_1_OPERATION

Description:
        - This test case verifies the functionality of the Spectrum Analyzer plugin on channel 1.

OS:
        - any

Steps:
        * Step 1: Test Channel 1’s frequency accuracy
        * Step 1.1: On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 1.2: Connect Scope ch1+ to W+ and Scope ch1- to GND
        * Step 2: Test at 500Hz
        * Step 2.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1kHz, set the Resolution BW to 244.14mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 500Hz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 2.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500Hz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 500Hz. The markers should also detect this as the peak amplitude.
        * Step 3: Test at 1kHz
        * Step 3.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2kHz, set the Resolution BW to 976.56mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 1kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 3.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 1kHz. The markers should also detect this as the peak amplitude.
        * Step 4: Test at 7.5kHz
        * Step 4.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 5 kHz and Stop frequency at 10kHz, set the Resolution BW to 4.88Hz. On signal Generator, Set Amplitude: 10V, Frequency: 7.5kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 4.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 7.5kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 7.5kHz. The markers should also detect this as the peak amplitude.
        * Step 5: Test at 100kHz
        * Step 5.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 200kHz, set the Resolution BW to 12.21Hz. On signal Generator, Set Amplitude: 10V, Frequency: 100kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 5.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 100 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 100kHz. The markers should also detect this as the peak amplitude.
        * Step 6: Test at 250 kHz
        * Step 6.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 kHz, set the Resolution BW to 30.52 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 6.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 250 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 250kHz. The markers should also detect this as the peak amplitude.
        * Step 7: Test at 500 kHz
        * Step 7.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1 MHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 7.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 500kHz. The markers should also detect this as the peak amplitude.
        * Step 8: Test at 800 kHz
        * Step 8.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1.6 MHz, set the Resolution BW to 98.44 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 800 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 8.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 800 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 800 kHz. The markers should also detect this as the peak amplitude.
        * Step 9: Test at 1 MHz
        * Step 9.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2 MHz, set the Resolution BW to 122.07 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 1 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 9.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 1 MHz. The markers should also detect this as the peak amplitude.
        * Step 10: Test at 5 MHz
        * Step 10.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 MHz, set the Resolution BW to 610.35 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 5 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 10.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 5 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 5 MHz. The markers should also detect this as the peak amplitude.
        * Step 11: Test at 10 MHz
        * Step 11.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 MHz, set the Resolution BW to 1.53 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 10 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 11.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 10 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 10 MHz. The markers should also detect this as the peak amplitude.
        * Step 12: Test at 20 MHz
        * Step 12.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 MHz, set the Resolution BW to 3.05 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 20 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 12.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 20 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 20 MHz. The markers should also detect this as the peak amplitude.

Test 2: Channel 2 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.SA.CHANNEL_2_OPERATION

Description:
        - This test case verifies the functionality of the Spectrum Analyzer plugin on channel 2.

OS:
        - any

Steps:
        * Step 1: Test Channel 2’s frequency accuracy
        * Step 1.1: On Channel 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 1.2: Connect Scope ch2+ to W2+ and Scope ch2- to GND
        * Step 2: Test at 500Hz
        * Step 2.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1kHz, set the Resolution BW to 244.14mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 500Hz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 2.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500Hz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 500Hz. The markers should also detect this as the peak amplitude.
        * Step 3: Test at 1kHz
        * Step 3.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2kHz, set the Resolution BW to 976.56mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 1kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 3.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 1kHz. The markers should also detect this as the peak amplitude.
        * Step 4: Test at 7.5kHz
        * Step 4.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 5 kHz and Stop frequency at 10kHz, set the Resolution BW to 4.88Hz. On signal Generator, Set Amplitude: 10V, Frequency: 7.5kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 4.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 7.5kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 7.5kHz. The markers should also detect this as the peak amplitude.
        * Step 5: Test at 100kHz
        * Step 5.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 200kHz, set the Resolution BW to 12.21Hz. On signal Generator, Set Amplitude: 10V, Frequency: 100kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 5.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 100 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 100kHz. The markers should also detect this as the peak amplitude.
        * Step 6: Test at 250 kHz
        * Step 6.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 kHz, set the Resolution BW to 30.52 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 6.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 250 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 250kHz. The markers should also detect this as the peak amplitude.
        * Step 7: Test at 500 kHz
        * Step 7.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1 MHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 7.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 500kHz. The markers should also detect this as the peak amplitude.
        * Step 8: Test at 800 kHz
        * Step 8.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1.6 MHz, set the Resolution BW to 98.44 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 800 kHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 8.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 800 kHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 800 kHz. The markers should also detect this as the peak amplitude.
        * Step 9: Test at 1 MHz
        * Step 9.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2 MHz, set the Resolution BW to 122.07 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 1 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 9.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 1 MHz. The markers should also detect this as the peak amplitude.
        * Step 10: Test at 5 MHz
        * Step 10.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 MHz, set the Resolution BW to 610.35 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 5 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 10.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 5 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 5 MHz. The markers should also detect this as the peak amplitude.
        * Step 11: Test at 10 MHz
        * Step 11.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 MHz, set the Resolution BW to 1.53 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 10 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 11.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 10 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 10 MHz. The markers should also detect this as the peak amplitude.
        * Step 12: Test at 20 MHz
        * Step 12.1: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 MHz, set the Resolution BW to 3.05 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 20 MHz, Offset: 0V and Phase: 0 degrees
                * Expected Results: The interface should look like in the “Step Resources” picture. After setting the start and stop frequency, the center frequency and Span should follow.
        * Step 12.2: Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 20 MHz Frequency position, or click the peak button for a shortcut.
                * Expected Results: The fundamental frequency should be on 20 MHz. The markers should also detect this as the peak amplitude.


Test 3: Channel 1 and 2 Operation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.SA.CHANNEL_1_AND_2_OPERATION

Description:
        - This test case verifies the functionality of the Spectrum Analyzer plugin on channel 1 and 2.

OS:
        - any

Steps:
        * Step 1: Testing the marker function for channel 1 and 2
        * Step 1.1: On channel 1 and 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 1.2: Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        * Step 1.3: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1MHz, set the Resolution BW to 61.04Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 750 kHz, Offset: 0V and Phase: 0 degrees
        * Step 1.4: Open the marker setting and select channel 1. Enable marker 1,2,3,4 or 5.
                * Expected Results: The marker is enabled when the number box is filled with color. The initial position of the marker is on the center frequency of the window.
        * Step 1.5: Click the peak button.
                * Expected Results: The marker highlighted should detect the fundamental frequency of the channel 1’s signal which is on 250kHz.
        * Step 1.6: Click the “→ peak” button.
                * Expected Results: The marker highlighted shouldn’t detect the fundamental frequency of the channel 2’s signal which is on 750kHz.
        * Step 1.7: Click the “Dn Ampl” button.
                * Expected Results: The marker should detect the next lower amplitude signal compared from the previous point within the channel 1’s spectrum.
        * Step 1.8: Click the “Up Ampl” button.
                * Expected Results: The marker should detect the next higher amplitude signal compared from the previous point within the channel 1’s spectrum.
        * Step 1.9: Open the marker setting and select channel 2. Enable marker 1,2,3,4 or 5.
                * Expected Results: The marker is enabled when the number box is filled with color. The initial position of the marker is on the center frequency of the window.
        * Step 1.10: Click the peak button.
                * Expected Results: The marker highlighted should detect the fundamental frequency of the channel 2’s signal which is on 750kHz.
        * Step 1.11: Click the “← peak” button.
                * Expected Results: The marker highlighted shouldn’t detect the fundamental frequency of the channel 1’s signal which is on 250kHz.
        * Step 1.12: Click the “Dn Ampl” button.
                * Expected Results: The marker should detect the next lower amplitude signal compared from the previous point within the channel 2’s spectrum.
        * Step 1.13: Click the “Up Ampl” button.
                * Expected Results: The marker should detect the next higher amplitude signal compared from the previous point within the channel 2’s spectrum.
        * Step 2: Testing channel 1 and 2 simultaneously
        * Step 2.1: On channel 1 and 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 2.2: Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        * Step 2.3: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 Hz, set the Resolution BW to 488.28 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 100 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 300 Hz, Offset: 0V and Phase: 0 degrees
        * Step 2.4: Run the Signal Generator and Spectrum Analyzer. Set Marker Table on to monitor marker values.
                * Expected Results: The fundamental frequency should be on 100 Hz for channel 1 and 300 Hz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.5: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.6: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1k Hz, set the Resolution BW to 976.56 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 200 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 600 Hz, Offset: 0V and Phase: 0 degrees
        * Step 2.7: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 200 Hz for channel 1 and 600 Hz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.8: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.9: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1k Hz, set the Resolution BW to 976.56 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 300 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 700 Hz, Offset: 0V and Phase: 0 degrees
        * Step 2.10: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 300 Hz for channel 1 and 700 Hz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.11: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.12: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 kHz, set the Resolution BW to 4.88 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 4 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 7k Hz, Offset: 0V and Phase: 0 degrees
        * Step 2.13: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 4 kHz for channel 1 and 7 kHz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.14: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.15: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 kHz, set the Resolution BW to 9.77 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 10 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 15 kHz, Offset: 0V and Phase: 0 degrees
        * Step 2.16: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 10 kHz for channel 1 and 15 kHz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.17: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.18: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 kHz, set the Resolution BW to 24.41 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 25 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 35 kHz, Offset: 0V and Phase: 0 degrees
        * Step 2.19: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 25 kHz for channel 1 and 35 kHz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.20: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.
        * Step 2.21: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 100 kHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 50 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 70 kHz, Offset: 0V and Phase: 0 degrees
        * Step 2.22: Run the Signal Generator and Spectrum Analyzer.
                * Expected Results: The fundamental frequency should be on 50 kHz for channel 1 and 70 kHz for channel 2. The signals shouldn’t be interfering the other.
        * Step 2.23: Repeat Testing the marker function for channel 1 and 2 from steps 1.4 to 1.13.
                * Expected Results: The behavior should be the same.


Test 4: Additional Features
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UID:
        - TST.M2K.SA.ADDITIONAL_FEATURES

Description:
        - This test case verifies the additional features of the Spectrum Analyzer plugin.

OS:
        - any

Steps:
        * Step 1: Testing channel 1’s trace detector type
        * Step 1.1: On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 1.2: Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        * Step 1.3: On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1MHz, set the Resolution BW to 61.04Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
        * Step 2: Test Peak hold Continuous
        * Step 2.1: On channel 1’s setting, set the detector type to Peak hold continuous. Run Spectrum Analyzer and Signal Generator.
                * Expected Results: The noise floor of the signal should move up to the peak of the noise floor.
        * Step 2.2: On Signal Generator’s channel 1, change the frequency to 250 kHz.
                * Expected Results: The signal should be able to capture the fundamental frequency at 250kHz while retaining the previous fundamental frequency from 500kHz signal
        * Step 3: Test Min hold Continuous
        * Step 3.1: Repeat the steps of testing detector types. On channel 1’s setting, set the detector type to Min hold continuous. Run Spectrum Analyzer and Signal Generator.
                * Expected Results: The noise floor of the signal should move down to the minimum value of the noise floor while retaining the fundamental frequency at 500kHz.
        * Step 3.2: On Signal Generator’s channel 1, change the frequency to 250 kHz.
                * Expected Results: The fundamental frequencies shouldn’t be detected but the noise floor’s should still be moving to the minimum
        * Step 4: Testing channel 2’s trace detector type
        * Step 4.1: Repeat the steps in channel 1's trace detector using channel 2.
                * Expected Results: The response should be the same
        * Step 5: Testing the marker table
        * Step 5.1: On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
                * Expected Results: The interface should look like in the “Step Resources” picture.
        * Step 5.2: Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        * Step 5.3: Set Signal Generator’s channel 1 to the following parameter: Waveformtype: Square Wave, Amplitude: 5V, Frequency: 50kHz, Offset: 0V and Phase 0 degrees. For channel 2 set the following parameters: Waveform type: Triangle , Amplitude: 5V, Frequency: 100kHz, offset: 0V and Phase: 0 degrees
        * Step 5.4: Set Spectrum Analyzer’s channel 1 and 2’s type to sample and Window to Flat top. For the Sweep setting set Start: 0Hz, Stop: 1MHz , Resolution BW: 61.04Hz. Run both Signal Generator and Spectrum Analyzer.
                * Expected Results: The spectrum analyzer now displays the FFT signal of both signals with the fundamental frequency and harmonics.
        * Step 5.5: On the marker menu, enable the marker table feature.
                * Expected Results: The interface should look like the image in the step resource picture.
        * Step 5.6: Enable 5 markers for the two channels and distribute each markers on the fundamental frequency or harmonic frequency of the signal by pressing “Up Ampl” or “Dn Ampl”
                * Expected Results: For channel 1 the fundamental frequency is on 50kHz and the succeeding harmonics are at 150kHz, 250kHz, 350kHz and 450kHz. For channel 2, the fundamental frequency is on 100kHz and the succeeding harmonics is on 300kHz, 500kHz, 700kHz and 900kHz. See Step resource picture for reference.
