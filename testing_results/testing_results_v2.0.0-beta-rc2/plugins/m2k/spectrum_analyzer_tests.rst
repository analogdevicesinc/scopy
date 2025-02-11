.. _m2k_spectrum_analyzer_tests:

Spectrum Analyzer - Test Suite
====================================================================================================

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
----------------------------------------------------------------------------------------------------

.. _m2k-usb-spectrum-analyzer:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Channel 1 Operation
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.SA.CHANNEL_1_OPERATION

**Description:** This test case verifies the functionality of the Spectrum Analyzer plugin on channel 1.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-spectrum-analyzer>` setup.

**Steps:**
        1. Test Channel 1’s frequency accuracy
        2. On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        3. Connect Scope ch1+ to W+ and Scope ch1- to GND
        4. Test at 500Hz
        5. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1kHz, set the Resolution BW to 244.14mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 500Hz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500Hz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500Hz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Test at 1kHz
        8. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2kHz, set the Resolution BW to 976.56mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 1kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Test at 7.5kHz
        11. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 5 kHz and Stop frequency at 10kHz, set the Resolution BW to 4.88Hz. On signal Generator, Set Amplitude: 10V, Frequency: 7.5kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 7.5kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 7.5kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Test at 100kHz
        14. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 200kHz, set the Resolution BW to 12.21Hz. On signal Generator, Set Amplitude: 10V, Frequency: 100kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        15. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 100 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 100kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        16. Test at 250 kHz
        17. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 kHz, set the Resolution BW to 30.52 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        18. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 250 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 250kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        19. Test at 500 kHz
        20. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1 MHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        22. Test at 800 kHz
        23. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1.6 MHz, set the Resolution BW to 98.44 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 800 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        24. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 800 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 800 kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        25. Test at 1 MHz
        26. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2 MHz, set the Resolution BW to 122.07 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 1 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        27. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        28. Test at 5 MHz
        29. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 MHz, set the Resolution BW to 610.35 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 5 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        30. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 5 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 5 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        31. Test at 10 MHz
        32. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 MHz, set the Resolution BW to 1.53 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 10 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        33. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 10 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 10 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        34. Test at 20 MHz
        35. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 MHz, set the Resolution BW to 3.05 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 20 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        36. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 20 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 20 MHz. The markers should also detect this as the peak amplitude.
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
--------------------------------------------

**UID:** TST.M2K.SA.CHANNEL_2_OPERATION

**Description:** This test case verifies the functionality of the Spectrum Analyzer plugin on channel 2.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-spectrum-analyzer>` setup.

**Steps:**
        1. Test Channel 2’s frequency accuracy
        2. On Channel 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        3. Connect Scope ch2+ to W2+ and Scope ch2- to GND
        4. Test at 500Hz
        5. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1kHz, set the Resolution BW to 244.14mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 500Hz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500Hz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500Hz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Test at 1kHz
        8. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2kHz, set the Resolution BW to 976.56mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 1kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Test at 7.5kHz
        11. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 5 kHz and Stop frequency at 10kHz, set the Resolution BW to 4.88Hz. On signal Generator, Set Amplitude: 10V, Frequency: 7.5kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 7.5kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 7.5kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Test at 100kHz
        14. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 200kHz, set the Resolution BW to 12.21Hz. On signal Generator, Set Amplitude: 10V, Frequency: 100kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        15. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 100 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 100kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        16. Test at 250 kHz
        17. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 kHz, set the Resolution BW to 30.52 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        18. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 250 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 250kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        19. Test at 500 kHz
        20. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1 MHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 500 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        22. Test at 800 kHz
        23. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1.6 MHz, set the Resolution BW to 98.44 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 800 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        24. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 800 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 800 kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        25. Test at 1 MHz
        26. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2 MHz, set the Resolution BW to 122.07 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 1 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        27. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        28. Test at 5 MHz
        29. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 MHz, set the Resolution BW to 610.35 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 5 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        30. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 5 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 5 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        31. Test at 10 MHz
        32. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 MHz, set the Resolution BW to 1.53 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 10 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        33. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 10 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 10 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

        34. Test at 20 MHz
        35. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 MHz, set the Resolution BW to 3.05 kHz. On signal Generator, Set Amplitude: 10V, Frequency: 20 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        36. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 20 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 20 MHz. The markers should also detect this as the peak amplitude.
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


Test 3: Channel 1 and 2 Operation
--------------------------------------------

**UID:** TST.M2K.SA.CHANNEL_1_AND_2_OPERATION

**Description:** This test case verifies the functionality of the Spectrum Analyzer plugin on channel 1 and 2.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-spectrum-analyzer>` setup.

**Steps:**
        1. Testing the marker function for channel 1 and 2
        2. On channel 1 and 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        3. Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        4. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1MHz, set the Resolution BW to 61.04Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 750 kHz, Offset: 0V and Phase: 0 degrees
        5. Open the marker setting and select channel 1. Enable marker 1,2,3,4 or 5.
                - **Expected Result:** The marker is enabled when the number box is filled with color. The initial position of the marker is on the center frequency of the window.
                - **Actual Result:**

..
  Actual test result goes here.
..

        6. Click the peak button.
                - **Expected Result:** The marker highlighted should detect the fundamental frequency of the channel 1’s signal which is on 250kHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Click the “→ peak” button.
                - **Expected Result:** The marker highlighted shouldn’t detect the fundamental frequency of the channel 2’s signal which is on 750kHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        8. Click the “Dn Ampl” button.
                - **Expected Result:** The marker should detect the next lower amplitude signal compared from the previous point within the channel 1’s spectrum.
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Click the “Up Ampl” button.
                - **Expected Result:** The marker should detect the next higher amplitude signal compared from the previous point within the channel 1’s spectrum.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Open the marker setting and select channel 2. Enable marker 1,2,3,4 or 5.
                - **Expected Result:** The marker is enabled when the number box is filled with color. The initial position of the marker is on the center frequency of the window.
                - **Actual Result:**

..
  Actual test result goes here.
..

        11. Click the peak button.
                - **Expected Result:** The marker highlighted should detect the fundamental frequency of the channel 2’s signal which is on 750kHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Click the “← peak” button.
                - **Expected Result:** The marker highlighted shouldn’t detect the fundamental frequency of the channel 1’s signal which is on 250kHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Click the “Dn Ampl” button.
                - **Expected Result:** The marker should detect the next lower amplitude signal compared from the previous point within the channel 2’s spectrum.
                - **Actual Result:**

..
  Actual test result goes here.
..

        14. Click the “Up Ampl” button.
                - **Expected Result:** The marker should detect the next higher amplitude signal compared from the previous point within the channel 2’s spectrum.
                - **Actual Result:**

..
  Actual test result goes here.
..

        15. Testing channel 1 and 2 simultaneously
        16. On channel 1 and 2’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        17. Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        18. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 Hz, set the Resolution BW to 488.28 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 100 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 300 Hz, Offset: 0V and Phase: 0 degrees
        19. Run the Signal Generator and Spectrum Analyzer. Set Marker Table on to monitor marker values.
                - **Expected Result:** The fundamental frequency should be on 100 Hz for channel 1 and 300 Hz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        20. Repeat Testing the marker function for channel 1 and 2 from steps 5. to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1k Hz, set the Resolution BW to 976.56 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 200 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 600 Hz, Offset: 0V and Phase: 0 degrees
        22. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 200 Hz for channel 1 and 600 Hz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        23. Repeat Testing the marker function for channel 1 and 2 from steps 5. to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        24. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1k Hz, set the Resolution BW to 976.56 mHz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 300 Hz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 700 Hz, Offset: 0V and Phase: 0 degrees
        25. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 300 Hz for channel 1 and 700 Hz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        26. Repeat Testing the marker function for channel 1 and 2 from steps 5 to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        27. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 kHz, set the Resolution BW to 4.88 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 4 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 7k Hz, Offset: 0V and Phase: 0 degrees
        28. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 4 kHz for channel 1 and 7 kHz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        29. Repeat Testing the marker function for channel 1 and 2 from steps 5 to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        30. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 kHz, set the Resolution BW to 9.77 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 10 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 15 kHz, Offset: 0V and Phase: 0 degrees
        31. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 10 kHz for channel 1 and 15 kHz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        32. Repeat Testing the marker function for channel 1 and 2 from steps 5 to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        33. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 kHz, set the Resolution BW to 24.41 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 25 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 35 kHz, Offset: 0V and Phase: 0 degrees
        34. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 25 kHz for channel 1 and 35 kHz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        35. Repeat Testing the marker function for channel 1 and 2 from steps 5 to 14.
                - **Expected Result:** The behavior should be the same.
                - **Actual Result:**

..
  Actual test result goes here.
..

        36. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 100 kHz, set the Resolution BW to 61.04 Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 50 kHz, Offset: 0V and Phase: 0 degrees. Amplitude: 10V, Frequency: 70 kHz, Offset: 0V and Phase: 0 degrees
        37. Run the Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The fundamental frequency should be on 50 kHz for channel 1 and 70 kHz for channel 2. The signals shouldn’t be interfering the other.
                - **Actual Result:**

..
  Actual test result goes here.
..

        38. Repeat Testing the marker function for channel 1 and 2 from steps 5 to 14.
                - **Expected Result:** The behavior should be the same.
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
--------------------------------------------

**UID:** TST.M2K.SA.ADDITIONAL_FEATURES

**Description:** This test case verifies the additional features of the Spectrum Analyzer plugin.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-spectrum-analyzer>` setup.

**Steps:**
        1. Testing channel 1’s trace detector type
        2. On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        3. Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        4. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1MHz, set the Resolution BW to 61.04Hz. On signal Generator, Set Channel 1’s Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
        5. Test Peak hold Continuous
        6. On channel 1’s setting, set the detector type to Peak hold continuous. Run Spectrum Analyzer and Signal Generator.
                - **Expected Result:** The noise floor of the signal should move up to the peak of the noise floor.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. On Signal Generator’s channel 1, change the frequency to 250 kHz.
                - **Expected Result:** The signal should be able to capture the fundamental frequency at 250kHz while retaining the previous fundamental frequency from 500kHz signal
                - **Actual Result:**

..
  Actual test result goes here.
..

        8. Test Min hold Continuous
        9. Repeat the steps of testing detector types. On channel 1’s setting, set the detector type to Min hold continuous. Run Spectrum Analyzer and Signal Generator.
                - **Expected Result:** The noise floor of the signal should move down to the minimum value of the noise floor while retaining the fundamental frequency at 500kHz.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. On Signal Generator’s channel 1, change the frequency to 250 kHz.
                - **Expected Result:** The fundamental frequencies shouldn’t be detected but the noise floor’s should still be moving to the minimum
                - **Actual Result:**

..
  Actual test result goes here.
..

        11. Testing channel 2’s trace detector type
        12. Repeat the steps in channel 1's trace detector using channel 2.
                - **Expected Result:** The response should be the same
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Testing the marker table
        14. On channel 1’s setting, set Type to Sample, Window Function to Flat-top and Averaging to 1.
        15. Connect Scope ch1+ to W1 and Scope ch1- to GND. Connect Scope ch2+ to W2 and Scope ch2- to GND
        16. Set Signal Generator’s channel 1 to the following parameter: Waveformtype: Square Wave, Amplitude: 5V, Frequency: 50kHz, Offset: 0V and Phase 0 degrees. For channel 2 set the following parameters: Waveform type: Triangle , Amplitude: 5V, Frequency: 100kHz, offset: 0V and Phase: 0 degrees
        17. Set Spectrum Analyzer’s channel 1 and 2’s type to sample and Window to Flat top. For the Sweep setting set Start: 0Hz, Stop: 1MHz , Resolution BW: 61.04Hz. Run both Signal Generator and Spectrum Analyzer.
                - **Expected Result:** The spectrum analyzer now displays the FFT signal of both signals with the fundamental frequency and harmonics.
                - **Actual Result:**

..
  Actual test result goes here.
..

        18. On the marker menu, enable the marker table feature.
                - **Expected Result:** The interface should look like the image in the step resource picture.
                - **Actual Result:**

..
  Actual test result goes here.
..

        19. Enable 5 markers for the two channels and distribute each markers on the fundamental frequency or harmonic frequency of the signal by pressing “Up Ampl” or “Dn Ampl”
                - **Expected Result:** For channel 1 the fundamental frequency is on 50kHz and the succeeding harmonics are at 150kHz, 250kHz, 350kHz and 450kHz. For channel 2, the fundamental frequency is on 100kHz and the succeeding harmonics is on 300kHz, 500kHz, 700kHz and 900kHz. See Step resource picture for reference.
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

