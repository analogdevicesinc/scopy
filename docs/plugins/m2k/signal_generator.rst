.. _signal_generator:

Signal Generator
================================================================================

  **Video guide:**

.. video:: https://www.youtube.com/watch?v=zWX7VnKDYq4  


General Description
---------------------------------------------------------------------

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_14-57-31.png
    :align: center

The signal generator instrument can be used to generate analog output from the 
M2K with user configurable parameters. It consists of 3 parts:

* Channel selector - enable/disable channels as well as each channel's control 
  panel.

* Signal Plot - visual representation of the signals

* Control Panel - change signal paramteres

The signal generator instrument can be used to generate analog output from the 
M2K with user configurable parameters. It consists of 3 parts:

* Channel selector - enable/disable channels as well as each channel's control 
  panel.

* Signal Plot - visual representation of the signals

* Control Panel - change signal paramteres


Constant signal only has one parameter which inputs the amplitude of the signal 
Waveform signal type can be one of the following

* Sine

* Square

* Triangle

* Trapezoidal

* Rising sawtooth

* Falling sawtooth

All signals have amplitude, offset, phase and frequency parameters. Selecting 
Square wave will also unlock the duty cycle parameter. Selecting trapezoidal 
waveform will disable the frequency, as this parameter will be computed from 
the other 4 values in the Timing category (Rise time, High time, Fall time, Low 
Time)

Buffer signal type takes a file as an input. The supported file types for 
buffered signals are:

* .bin - 32 bit binary float format

* .wav - Waveform Audio File Format (16 bit integers)

* .csv - Comma separated values

* .mat - MATLAB Mat format

CSV file format supports raw CSVs such as `this <https://gist.github.com/adisuciu/7aa30bc9e545db23a17e86d23ae4f53c>`__ , 
as well as Scopy formatted CSVs such as `this <https://gist.github.com/adisuciu/5abffa8233707c7b95585e80fbb1dde9>`__ 
This means it is possible to acquire a signal in the oscilloscope and play it back in the signal generator.


This in not entirely true. The scope sample rate is 100 MSPS (or 
factors of ten less) and the signal generator sample rate is 75MSPS (or factors 
of 10 less) so scope samples do not inherently match up with generator samples. 
Scope samples would need to be re-sampled by a factor of 3 to 4 to have the 
signal frequency played back properly.

MAT file format only supports arrays of type real (no complex waveforms)


Math signal type allows generation of a signal that is defined as a math 
equation. .


On top of all the signals noise can be added. By selecting None, no noise is 
added to the signal. The rest of the noise types are:

* Uniform

* Gaussian

* Laplacian

* Impulse

The noise is only calculated once on the host, and simply added to the waveform 
buffer. This means that for cyclic waveforms the noise will be the same on each 
period.



**Use cases**
---------------------------------------------------------------------


**Create loopback between CH1 and CH2 of the oscilloscope and the signal generator**
-------------------------------------------------------------------------------------

  1.Disable channel 2

  2.Select sine wave with 5V amplitude and 10kHz

  3.Run the signal generator

  4.Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-25-33.png

|

**Run both channels**
---------------------------------------------------------------------

  1.Continue from previous testcase

  2.Enable channel 2 and select triangle wave with 5V amplitude and 20kHz

  3.Add gaussian noise with 1V amplitude

  4.Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-30-13.png

|

**Generate square waveform**
---------------------------------------------------------------------

  1.Select square wave with 25% dutycycle

  2.Decrease noise amplitude to 200mV

  3.Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-33-14.png

|

**Generate trapezoidal waveform**
---------------------------------------------------------------------

  1.Select trapezoidal waveform with 1ms rise/up/fall/low times

  2.Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-34-52.png

|

**Generate waveform from wav file**
---------------------------------------------------------------------

  1.Select buffer mode and select a wavefile. One can usually find a 
    suitable wavefile in C:\Windows\Media

  2.Signal generator automatically selects appropriate sample rate

  3.Monitor oscilloscope (if possible connect a speaker to the channel 
    that is outputted)

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-36-05.png

|

**Generate stairstep waveform**
---------------------------------------------------------------------

  1.Select the stairstep csv file provided above 
    https://gist.github.com/adisuciu/7aa30bc9e545db23a17e86d23ae4f53c

  2.Remove noise, disable CH1 and increase amplitude to 5V

  3.Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/scopy_2018-05-16_17-38-32.png

|

**Generate waveform from math function**
---------------------------------------------------------------------

  1. Select Math mode and input a function such as 
     2*(cos(6000*pi*t)*sin(2000*pi*t))

  2. Set sample rate to 75MSPS (this is the sample rate of the generated 
     signal)
  
  3. Set record run length to 1ms.
  
  4. Run the signal
  
  5. Monitor oscilloscope

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/signalGenerator/mathgenerator.png

|

.. note::
     Sample rate & record length parameters: Since t goes from 0 to 
     infinity, we need to specify how long will t be generated for the specified 
     function - in this case it will go from 0 to 1ms with 75 MSPS 
     granularity(13.3333 ns). These settings will generate (1 * 10^-3) * (75 * 10 
     ^9) points = 75000 points.

