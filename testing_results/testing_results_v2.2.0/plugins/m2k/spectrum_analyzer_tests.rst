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

**RBP:** P0

**Description:** This test case verifies the functionality of the Spectrum Analyzer plugin on channel 1.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-spectrum-analyzer>` setup.

**Steps:**
        1. Open Spectrum Analyzer plugin.
        2. On channel 1’s setting, set Type to Sample, Window Function to Flat-top and CH to 1.
        3. Connect Scope ch1+ to W+ and Scope ch1- to GND
        4. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1kHz, set the Resolution BW to 244.14mHZ. On signal Generator, Set Amplitude: 10V, Frequency: 500Hz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        5. Run the Signal Generator and Spectrum Analyzer. 
        6. On the markers menu, Enable Marker 1and set it manually at 500Hz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500Hz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 1kHz
      
        7. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2kHz, 
        8. Set the Resolution BW to 976.56mHZ. 
        9. On signal Generator, Set Amplitude: 10V, Frequency: 1kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 7.5kHz
      
        11. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 5 kHz and Stop frequency at 10kHz, set the Resolution BW to 4.88Hz. 
        12. On signal Generator, Set Amplitude: 10V, Frequency: 7.5kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Run the Signal Generator and Spectrum Analyzer. 
        14. On the markers menu, Enable Marker 1and set it manually at 7.5kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 7.5kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 100kHz
      
        15. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 200kHz, set the Resolution BW to 12.21Hz. 
        16. On signal Generator, Set Amplitude: 10V, Frequency: 100kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        17. Run the Signal Generator and Spectrum Analyzer. 
        18. On the markers menu, Enable Marker 1and set it manually at 100 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 100kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 250 kHz
      
        19. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 500 kHz, set the Resolution BW to 30.52 Hz. 
        20. On signal Generator, Set Amplitude: 10V, Frequency: 250 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. Run the Signal Generator and Spectrum Analyzer. 
        22. On the markers menu, Enable Marker 1and set it manually at 250 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 250kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 500 kHz
      
        23. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1 MHz, set the Resolution BW to 61.04 Hz. 
        24. On signal Generator, Set Amplitude: 10V, Frequency: 500 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        25. Run the Signal Generator and Spectrum Analyzer. 
        26. On the markers menu, Enable Marker 1and set it manually at 500 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 500kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 800 kHz
      
        27. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 1.6 MHz, set the Resolution BW to 98.44 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 800 kHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        28. Run the Signal Generator and Spectrum Analyzer. 
        29. On the markers menu, Enable Marker 1and set it manually at 800 kHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 800 kHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 1 MHz
      
        30. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 2 MHz, set the Resolution BW to 122.07 Hz. 
        31. On signal Generator, Set Amplitude: 10V, Frequency: 1 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        32. Run the Signal Generator and Spectrum Analyzer. On the markers menu, Enable Marker 1and set it manually at 1 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 1 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 5 MHz
      
        33. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 10 MHz, set the Resolution BW to 610.35 Hz. On signal Generator, Set Amplitude: 10V, Frequency: 5 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        34. Run the Signal Generator and Spectrum Analyzer. 
        35. On the markers menu, Enable Marker 1and set it manually at 5 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 5 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 10 MHz
      
        36. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 20 MHz, set the Resolution BW to 1.53 kHz. 
        37. On signal Generator, Set Amplitude: 10V, Frequency: 10 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        38. Run the Signal Generator and Spectrum Analyzer. 
        39. On the markers menu, Enable Marker 1and set it manually at 10 MHz Frequency position, or click the peak button for a shortcut.
                - **Expected Result:** The fundamental frequency should be on 10 MHz. The markers should also detect this as the peak amplitude.
                - **Actual Result:**

..
  Actual test result goes here.
..

      Test at 20 MHz
      
        40. On Spectrum Analyzer’s Sweep setting, Set Start Frequency at 0Hz and Stop frequency at 50 MHz, set the Resolution BW to 3.05 kHz. 
        41. On signal Generator, Set Amplitude: 10V, Frequency: 20 MHz, Offset: 0V and Phase: 0 degrees
                - **Expected Result:** After setting the start and stop frequency, the center frequency and Span should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        42. Run the Signal Generator and Spectrum Analyzer. 
        43. On the markers menu, Enable Marker 1and set it manually at 20 MHz Frequency position, or click the peak button for a shortcut.
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


