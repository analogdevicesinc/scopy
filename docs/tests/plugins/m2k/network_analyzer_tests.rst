.. _network_analyzer_tests:

Network Analyzer - Test Cases
=============================

Initial Setup
-------------

In order to proceed through the test case, first of all delete the Scopy \*.ini file (saves previous settings made in Scopy tool) from the following path on Windows: `C:\Users\your_username\AppData\Roaming\ADI`.

Open the Network Analyzer instrument. The interface should look like the picture below:

Press multiple times on the “Run” button to check if the instrument works.

Test 1 - Low Pass Filter Test
-----------------------------

Low Pass Filter: fc = 340Hz

1.1 Create a first-order low pass RC filter with the following components: R = 470 Ohms, C = 1uF. This will have a cut-off frequency of approximately 340 Hz.
    - The set up must look like the image in the Steps Resources picture on the left.

1.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 10Hz, Stop – 500kHz, Sample Count – 100
    - c. Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -150°, Max. Phase – 60°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left.

1.3 Enable the “Cursor” by clicking the box at the lower right corner of the interface.
    - The interface must look like the image in the Steps Resources picture.

1.4 Move the cursor from left to right.
    - The values displayed on the screen must follow accordingly.

1.5 Move the cursor to find the -3dB point on the trace.
    - The magnitude must indicate -3dB at approximately 340 Hz. The phase corresponding to the same frequency must also be indicated.

1.6 Disable the cursor by clicking the box again.
    - The cursor controls must disappear from the interface.

1.7 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -70dB, Max. Magnitude – 50dB, Min. Phase – -120°, Max. Phase – 60°
    - The plot must change accordingly as you change the values by either using the buttons or by entering the values. The plot must now look like the image in the Steps Resources.

1.8 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

1.9 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 340 Hz.

Reference: CH2
~~~~~~~~~~~~~~

1.10 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2 and then repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Low Pass Filter: fc = 15.9kHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.1 Create a first-order low pass RC filter with the following components: R = 100 Ohms, C = 0.1uF. This will have a cut-off frequency of approximately 15.9kHz.
    - The set up must look like the image in the Steps Resources picture on the left. The frequency response may change due to the type of components used.

Reference: CH1
~~~~~~~~~~~~~~

2.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 10Hz, Stop – 1MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -70dB, Max. Magnitude – 5dB, Min. Phase – -120°, Max. Phase – 60°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable and move your cursor to find the -3dB point of your trace. It must be around 15.9 kHz.

2.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -45dB, Max. Magnitude – 10dB, Min. Phase – -100°, Max. Phase – 20°
    - The plot must now look like image in the Steps Resources. The plot must change accordingly as you change the values.

2.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

2.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 15.9 kHz.

Reference: CH2
~~~~~~~~~~~~~~

2.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 10Hz to 1MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Low Pass Filter: fc = 1.59MHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3.1 Create a first-order low pass RC filter with the following components: R = 1 kOhms, C = 100 pF. This will have a cut-off frequency of approximately 1.59MHz.
    - The set up must look like the image in the Steps Resources picture on the left. The frequency response may change due to the type of components used.

Reference: CH1
~~~~~~~~~~~~~~

3.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 50Hz, Stop – 30MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -45dB, Max. Magnitude – 10dB, Min. Phase – -100°, Max. Phase – 20°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable and move your cursor to find the -3dB point of your trace. It must be around 1.59MHz.

3.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -35dB, Max. Magnitude – 5dB, Min. Phase – -100°, Max. Phase – 35°
    - The plot must change accordingly as you change the values.

3.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the image in the Steps Resources.

3.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 1.59 MHz.

Reference: CH2
~~~~~~~~~~~~~~

3.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 50Hz to 30MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.


Test 2 - High Pass Filter Test
------------------------------

High Pass Filter: fc = 340 Hz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1.1 Create a first-order high pass RC filter with the following components: R = 470 Ohms, C = 1uF. This will have a cut-off frequency of approximately 340 Hz.
    - The set up must look like the image in the Steps Resources picture on the left.

Reference: CH1
~~~~~~~~~~~~~~

1.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 1Hz, Stop – 1MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -180°, Max. Phase – 180°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 340 Hz.

1.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -40dB, Max. Magnitude – 5dB, Min. Phase – -100°, Max. Phase – 150°
    - The plot must change accordingly as you change the values by either using the buttons or by entering the values. The plot must now look like the image in the Steps Resources.

1.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images in the Steps Resources.

1.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 340 Hz.

Reference: CH2
~~~~~~~~~~~~~~

1.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2 and then repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

High Pass Filter: fc = 15.9kHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.1 Create a first-order high pass RC filter with the following components: R = 100 Ohms, C = 0.1uF. This will have a cut-off frequency of approximately 15.9kHz.
    - The set up must look like the image in the Steps Resources picture on the left. The frequency response may change due to the type of components used.

Reference: CH1
~~~~~~~~~~~~~~

2.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 100Hz, Stop – 10MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -70dB, Max. Magnitude – 5dB, Min. Phase – -100°, Max. Phase – 150°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable and move your cursor to find the -3dB point of your trace. It must be around 15.9 kHz.

2.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -50dB, Max. Magnitude – 5dB, Min. Phase – -180°, Max. Phase – 150°
    - The plot must change accordingly as you change the values by either using the buttons or by entering the values. The plot must now look like the image in the Steps Resources.

2.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images in the Steps Resources.

2.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 15.9 kHz.

Reference: CH2
~~~~~~~~~~~~~~

2.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2 and then repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

High Pass Filter: fc = 1.6MHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

3.1 Create a first-order high pass RC filter with the following components: R = 1 kOhms, C = 100pF. This will have a cut-off frequency of approximately 1.6MHz.
    - The set up must look like the image in the Steps Resources picture on the left. The frequency response may change due to the type of components used.

Reference: CH1
~~~~~~~~~~~~~~

3.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 10Hz, Stop – 30MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -70dB, Max. Magnitude – 5dB, Min. Phase – -120°, Max. Phase – 60°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable and move your cursor to find the -3dB point of your trace. It must be around 1.6MHz.

3.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -60dB, Max. Magnitude – 5dB, Min. Phase – -150°, Max. Phase – 180°
    - The plot must change accordingly as you change the values. The plot must now look like the image in the Steps Resources.

3.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the image in the Steps Resources.

3.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move it to find the -3dB point of your trace. It must be around 1.6MHz.

Reference: CH2
~~~~~~~~~~~~~~

3.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 10Hz to 1MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Test 3 - Band Pass Filter Test
------------------------------

Band Pass Filter: fcl = 1.59kHz, fch = 15.9kHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1.1 Create a first-order band pass RC filter with the following components: R1 = 100 Ohms, C1 = 1uF, R2 = 100 Ohms, C2 = 0.1uF. This will have a low cut-off frequency at around 1.59kHz and a high cut-off frequency of approximately 15.9 kHz.
    - The set up must look like the image in the Steps Resources picture on the left.

Reference: CH1
~~~~~~~~~~~~~~

1.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 50Hz, Stop – 5MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -90dB, Max. Magnitude – 10dB, Min. Phase – -180°, Max. Phase – 180°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable and move your cursor to find the -3dB points of your trace. It must be around 1.59 kHz and 15.9 kHz.

1.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -50dB, Max. Magnitude – 5dB, Min. Phase – -180°, Max. Phase – 180°
    - The plot must now look like the image in the Steps Resources. The plot must change accordingly as you change the values.

1.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

1.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move them to find the -3dB points of your trace. They must be around 1.59kHz and 15.9kHz.

Reference: CH2
~~~~~~~~~~~~~~

1.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 10Hz to 1MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Band Pass Filter: fcl = 3.4kHz, fch = 723kHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.1 Create a first-order band pass RC filter with the following components: R1 = 470 Ohms, C1 = 0.1uF, R2 = 2.2 kOhms, C2 = 100pF. This will have a low cut-off frequency at around 3.4kHz and a high cut-off frequency of approximately 723kHz.
    - The set up must look like the image in the Steps Resources picture on the left. The frequency response may change due to the type of components used.

Reference: CH1
~~~~~~~~~~~~~~

2.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 50Hz, Stop – 10MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -50dB, Max. Magnitude – 5dB, Min. Phase – -180°, Max. Phase – 180°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable the “Cursor” and move them to find the -3dB points of your trace. They must be around 3.4 kHz and 723 kHz.

2.3 Change the “Display” settings’ parameters to view the plot effectively by clicking ± button or by entering these values:
    - Display: Min. Magnitude – -40dB, Max. Magnitude – 5dB, Min. Phase – -150°, Max. Phase – 150°
    - The plot must change accordingly as you change the values by either using the buttons or by entering the values. The plot must now look like the image in the Steps Resources.

2.4 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

2.5 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings.
    - The plot must look like the image in the Steps Resources. Enable the “Cursor” and move them to find the -3dB points of your trace. They must be around 3.4 kHz and 723 kHz.

Reference: CH2
~~~~~~~~~~~~~~

2.6 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2 and then repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Band Stop Filter Test
---------------------

Notch Filter: fN = 795Hz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

1.1 Create a band stop notch filter with the following components: R1 = 100 Ohms, 2xC1 = 1uF, 2xR2 = 200 Ohms, C2 = 2uF. For C2, since it is not a standard value, you might use a capacitor box or connect two 1uF capacitors in parallel to achieve the value of the capacitance. This will have a notch frequency of approximately 795Hz. At this frequency, it will be highly attenuated.
    - The set up must look like the image in the Steps Resources picture on the left.

Reference: CH1
~~~~~~~~~~~~~~

1.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 20Hz, Stop – 10MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -50dB, Max. Magnitude – 5dB, Min. Phase – -180°, Max. Phase – 180°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable the “Cursor” and move it to find the lowest point of your trace. It must be around 795Hz where the frequency is highly attenuated.

1.3 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

1.4 Change the plot back to Bode plot and switch to “Linear” frequency scale by selecting Linear under frequency sweep settings. Change the sweep’s stop frequency to 10kHz by using the ± button.
    - The plot must look like the image in the Steps Resources.

Reference: CH2
~~~~~~~~~~~~~~

1.5 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 10Hz to 1MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.

Notch Filter: fN = 16.9kHz
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

2.1 Create a band stop notch filter with the following components: R1 = 100 Ohms, 2xC1 = 0.047uF, 2xR2 = 200 Ohms, C2 = 0.094uF. For C2, since it is not a standard value, you might use a capacitor box or connect two 0.047uF capacitors in parallel to achieve the value of the capacitance. This will have a notch frequency of approximately 16.9kHz. At this frequency, it will be highly attenuated.
    - The set up must look like the image in the Steps Resources picture on the left.

Reference: CH1
~~~~~~~~~~~~~~

2.2 Set these configurations on the Network Analyzer:
    - a. Reference: Channel 1, 1V Amplitude, 0V Offset
    - b. Sweep: Logarithmic, Start – 20Hz, Stop – 10MHz, Sample Count – 100
    - c. Display: Min. Magnitude – -50dB, Max. Magnitude – 5dB, Min. Phase – -180°, Max. Phase – 180°
    - d. Run the Network Analyzer.
    - Check the frequency response in the Bode plot. It must look like the image in the Steps Resources picture on the left. Enable the “Cursor” and move it to find the lowest point of your trace. It must be around 16.9kHz where the frequency is highly attenuated.

2.3 View the Nyquist and Nichols plots by going to the General Settings and choose the type of plot by clicking the dropdown box: “Nyquist” or “Nichols” (aside from “Bode”).
    - The plot must look like the images from the Steps Resources.

2.4 View back to Bode plot. Switch to Linear frequency scale by selecting Linear under frequency sweep settings. Change the sweep’s stop frequency to 200kHz by using the ± button.
    - The plot must look like the image in the Steps Resources.

Reference: CH2
~~~~~~~~~~~~~~

2.5 Change the placement of the scope channels (please see the picture on the left for the setup). Switch the reference channel to Channel 2. Set these sweep values: 10Hz to 1MHz. Repeat the test steps done with CH1.
    - The results must be the same with CH1’s.



Test 4 - Print Plot
-------------------

1.1 Repeat the steps 1.1 and 1.2 from Section A - Low Pass Filter. Press the “Print” button and save the file.
    - The resulted file must correspond to the plot presented in the resources column.

Test 5 - Buffer Previewer
-------------------------

2.1 Repeat Step 1.1 from Section A - Low Pass Filter and set the configurations from Step 1.2. Change the sweep’s start and stop frequencies to 10Hz and 5kHz respectively.
    - The set up must look like image from the Steps Resources picture on the left.

2.2 Turn the Buffer Previewer “on” and click “Run.”
    - The waveform above the plot must portray the current frequency being read. The Sample Count, Current Frequency, and the instantaneous DC Voltage must all start simultaneously. Check frequency response in Bode plot, the plot must look like in steps resources picture.

2.3 Slide the blue slider from the start to the end of the Bode Plot.
    - The waveform changes with regards to the Bode Plot.

2.4 Place the blue slider on the 1kHz mark. Run the Network Analyzer.
    - The waveform will stop displaying the instantaneous frequency at 1kHz. The sample count, current frequency, and DC voltage will continue to display instantaneous readings.

Test 6 - Gain Mode
------------------

3.1 Under Response in the Settings, change the Gain Mode from “Automatic” to “High.” Run network analyzer and then “Print” and save plot as “high_lpf.”
    - Check the frequency response in the Bode plot and it must look like the images from the Steps Resources.

3.2 Back in Settings, change the Gain Mode to “Low.” Run the Network Analyzer. “Print” and save the plot as “low_lpf.”
    - Check frequency response in Bode plot, the plot should look like in steps resources picture.

3.3 Compare the two saved plots - the “high_lpf” plot and the “low_lpf” plot.
    - The plots with gain mode set to Low have more spikes than that of the plots with gain mode set to High.

3.4 Change the Gain Mode from “Low” to “Automatic.” Run the Network Analyzer and observe the plot.
    - Scopy would use the appropriate gain mode for the circuit. In this case, the plot must be similar to that of the High Gain Mode.

Test 7 - DC Filtering, Off
--------------------------

4.1 Set the offset to 1V and the gain mode to High. Run the Network Analyzer.
    - The plot must be similar to the image in the Step Resource.

4.2 Set the sweep to “Linear.” Run the Network Analyzer.
    - The plot must be similar to that of the Step Resource picture. It must have a jagged and noisy plot.

Test 8 - DC Filtering, On
-------------------------

4.3 Turn DC Filtering “On” and set the sweep back to “Logarithmic.” Click Run.
    - The plot must be similar to that of the Step Resource picture. It must be smoother than the plot in step 4.1.

4.4 Set the sweep to “Linear.” Run the Network Analyzer.
    - The plot must be similar to that of the Step Resource picture. It must be less spiky than its step 4.2 counterpart.


Test 9 - Reference: Export/Import
---------------------------------

5.1 Repeat Step 1.1 from Section B - High Pass Filter and set the configurations from Step 1.2. Turn the Buffer Previewer “On” and run “Single” on the Network Analyzer.
    - The setup and plot must look like the images from the Steps Resources.

5.2 Click the General Settings button (the one that looks like a gear) and “Export” the plot. A dialog box will open, set “automatic_hpf” as its filename and click “Save.”
    - The plot must look identical to the Step Resource picture.

5.3 In Settings, change the Gain Mode from “Automatic” to “Low” and run the Network Analyzer.
    - See the Step Resource picture for reference.

5.4 Back in the General Settings, click “Import” and select the file you exported earlier.
    - Waveforms in red will be superimposed on the Bode Plot both to the Magnitude and Phase. It will serve as a reference for future plots.

5.5 To remove the imported reference waveform, click “Remove Reference.”
    - The waveforms in red will disappear. See step resource picture for reference.

Test 10 - Reference: Snapshot
------------------------------

6.1 Repeat Step 1.1 from Section B - High Pass Filter and set the configurations from Step 1.2. Turn the Buffer Previewer “On” and run “Single” on the Network Analyzer.
    - The plot must look similar to the step resource picture.

6.2 On General Settings, click “Snapshot.”
    - Waveforms in red will be superimposed on the Bode Plot both to the Magnitude and Phase. It will serve as a reference for future plots.

6.3 Go back to Settings and change the Gain Mode to “Low.” Run the Network Analyzer.
    - The plot readings will be superimposed on the reference waveform from Snapshot. This will make it easier to compare waveforms.