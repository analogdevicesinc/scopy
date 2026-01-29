.. _m2k_pattern_generator_tests:

Pattern Generator - Test Suite
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

.. _m2k-usb-pattern-generator:

**M2k.Usb:**
        - Open Scopy.
        - Connect an **ADALM2000** device to the system by USB.
        - Add the device in device browser.

Test 1: Individual Channel Operation
----------------------------------------------------------------------------------------------------

**UID:** TST.M2K.PG.INDIVIDUAL_CHANNEL_OPERATION

**RBP:** P0

**Description:** This test case verifies the functionality of the pattern generator in individual channel operation mode.

**Preconditions:**
        - Scopy is installed on the system.
        - OS: ANY
        - Use :ref:`M2k.Usb <m2k-usb-pattern-generator>` setup.

**Steps:**
        1. Checking Individual Channels: Use PP as output
        2. Connect DIO-0, CH0 to Scope CH1+, GND to Scope CH1-.
        3. Enable CH0. 
        4. Double click on the DIO 0 indicator on the plot to open DIO 0 settings. 
        5. Select pattern as Clock with a 5 KHz clock signal with duty cycle of 50%. 
        6. Run instrument.
                - **Expected Result:** You should see a square wave with 5 KHz frequency, 50% duty cycle.
                - **Actual Result:**

..
  Actual test result goes here.
..

        7. Monitor CH0 through oscilloscope. 
        8. Open built-in measurement feature for frequency, amplitude and duty cycle.
                - **Expected Result:** You should see a square wave with 5 KHz frequency, 50% duty cycle. In the oscilloscope, Frequency: 5 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 50 %, Duty-:50%
                - **Actual Result:**

..
  Actual test result goes here.
..

        9. Change frequency: 100 KHz, duty cycle: 30%.
                - **Expected Result:** You should see a square wave with 100 KHz frequency, 30% duty cycle. In the oscilloscope, Frequency: 100 KHz, Amplitude: 3.2V to 3.4 V, Duty+: 30 %, Duty-: 70%
                - **Actual Result:**

..
  Actual test result goes here.
..

        10. Change frequency: 1 MHz, duty cycle: 60%.
                - **Expected Result:** You should see a square wave with 1 MHz frequency, 60% duty cycle. In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 60 %, Duty-: 40%
                - **Actual Result:**

..
  Actual test result goes here.
..

        11. Change frequency: 10 MHz, duty cycle: 70%.
                - **Expected Result:** You should see a square wave with 10 MHz frequency, 70% duty cycle. In the oscilloscope, Frequency: 1 MHz, Amplitude: 3.2V to 3.4 V, Duty+: 70 %, Duty-: 30%
                - **Actual Result:**

..
  Actual test result goes here.
..

        12. Repeat steps 2. to 7. for DIO-1 to DIO-15.
                - **Expected Result:** Behavior of each channel should be the same as with DIO-0, CH0. As the parameters are changed, the trace displayed in oscilloscope should follow.
                - **Actual Result:**

..
  Actual test result goes here.
..

        13. Checking Phase
        14. Connect the following: 

          - DIO0 to ScopeCH1+,
          - DIO1 to ScopeCH2+,
          - GND to Scope CH1- and Scope CH2-.
        
        15. Enable DIO0 and DIO1. Set the following parameters: 
        
          - DIO0: Frequency: 5 KHz, Phase: 0°, Duty Cycle: 50%; 
          - DIO1: Frequency: 5 KHz, Phase: 45°, Duty Cycle: 50 %. 
        
        16. Run instrument.
                - **Expected Result:** You should see two square waves with 5 KHz frequency, 50% duty cycle. DIO0 should be at 0° phase and DIO1 should be at 45° phase.
                - **Actual Result:**

..
  Actual test result goes here.
..

        17. Monitor pattern generator output through oscilloscope.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 24us to 26us, corresponding to the 45° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        18. Change DIO1 phase: 120°.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 65us to 67us, corresponding to the 120° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        19. Change DIO1 phase: 270°.
                - **Expected Result:** Use cursor feature of the oscilloscope. Move the vertical cursors as shown in steps resources. ΔT = 149us to 151us, corresponding to the 270° phase shift.
                - **Actual Result:**

..
  Actual test result goes here.
..

        20. Set DIO1 phase to 0°. Now repeat steps for DIO0.
                - **Expected Result:** Behavior of channel should be the same as with DIO1, CH1.
                - **Actual Result:**

..
  Actual test result goes here.
..

        21. Use other channels, DIO2 to DIO15, and repeat steps to verify each.
                - **Expected Result:** Behavior of each channel should be the same as with DIO1, CH1.
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


