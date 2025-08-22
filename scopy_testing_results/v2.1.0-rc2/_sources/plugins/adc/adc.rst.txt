.. _adc:

ADC plugin
================================================================================

Description:

The ADC plugin is used to interface with IIO ADCs that implement an IIO buffer mechanism. The plugin implements two instruments for data acquisition and visualization in time and frequency domain.

Compatible:

The plugin is compatible with contexts that have at least an IIO device that implements an IIO buffer interface.

Usage:

ADC - Time instrument:

Plot Controls

X-Axis Menu

- Buffer Size - size of the acquisition buffer in samples

- Plot Size - size of the plot in samples. Can only be greater than the buffer size.

- Sync buffer-plot size - makes buffer and plot sizes equal

- Rolling Mode - controls how the plot is refreshed upon receiving a new buffer. When rolling mode is enabled, the buffers are placed in a queue, so the oldest buffer at the end of the queue is discarded in favor of the newest buffer at the beginning.

- X Min-Max control - sets the x axis limits

- X Mode - controls X axis units

  - Samples - On the X axis the index of each buffer is represented
  - Time -  sample rate is automatically read from the device if a sampling rate attribute is present
  - Time - override sample rate: allows the user to set the sampling rate

- Sample rate - The value of the sampling rate used in X axis computation, depending on the X Mode it can be readonly or can be overriden

Y-Axis Menu

- Y Min-Max control - sets the y axis limits

- Autoscale - disables Y axis controls and activates Y axis autoscaling

- Y Mode - controls Y axis units

 - ADC counts - raw values as read from the ADC
 - % Full scale - scales the input range to +/- 1.0
 - Scale - only available if scale parameter is present in the ADC channel and uses the value for scaling via SI unit scaling

XY plot - activates XY plotting

- X axis sources

- Plot X source 

Settings

- Plot name - Customize the name of the plot. It is shown on the plot itself
- Show plot labels - shows axis labels on the plot
- Thickness / Style - customize plot curve styling


Channel controls

- Y-Axis controls - separates the channel from the plot Y axis to it's own Axis with it's own set of controls

- Measurements - activates time domain measurements

- Curve - customize plot curve styling

- Attributes - IIO attributes that control the device

- Plot - which plot is the curve displayed on


Device controls:

- Attributes - IIO attributes that control the device


Cursors

- Enable - enable cursors on the plot

- Lock - lock cursors moving them together

- Track - track Y values on the X c cursor

- Move readouts - enables drag and drop of the cursor readouts on the plot

Measurements

- enables measurement/stats panel

- show all - enables all available Measurements

- Sort by type/channel


Frequency
Plot Controls

X-Axis Menu
- Buffer Size - size of the acquisition buffer in samples

- X Min-Max control - sets the x axis limits

- X Mode - controls X axis units

  - Samples - On the X axis the index of each buffer is represented
  - Time -  sample rate is automatically read from the device if a sampling rate attribute is present
  - Time - override sample rate: allows the user to set the sampling rate

- Sample rate - The value of the sampling rate used in X axis computation, depending on the X Mode it can be readonly or can be overriden

- Frequency offset - offsets X axis by some value

Y-Axis Menu

- Y Min-Max control - sets the y axis limits

- Autoscale - disables Y axis controls and activates Y axis autoscaling

- Power offset - offsets y axis by some amount

- Window - selects window used in the FFT

- Window correction - applies a correction in the FFT chain based on the Window

Settings

- Plot name - Customize the name of the plot. It is shown on the plot itself
- Show plot labels - shows axis labels on the plot
- Thickness / Style - customize plot curve styling


Channel Controls
- Y-Axis controls - separates the channel from the plot Y axis to it's own Axis with it's own set of controls

- Marker controls - enables, sets number of markers and selects between marker types
  - Fixed - fixed frequency marker
  - Peak - detects the first N peaks in the spectrum
  - Single Tone - detects DC, fundamental and the next N harmonics
  - Image - only available in complex mode - shows the DC, the fundamental and the fundamental image


- Curve - customize plot curve styling

- Attributes - IIO attributes that control the device

- Plot - which plot is the curve displayed on

Complex mode - activates complex mode. Complex channels are treated as whole different channels

.. _fft_averaging:

FFT Averaging (Frequency Instrument)
-------------------------------------

Starting with version 2.1.0, the ADC Frequency instrument supports FFT averaging using a 
moving average filter. This feature allows you to smooth the frequency spectrum by averaging 
multiple FFT frames, reducing noise fluctuations and making persistent signals easier to observe.

**How to use:**
- Open the channel menu in the Frequency instrument.
- Adjust the "Averaging Size" spinbox to set the number of FFT frames to average.
- The spectrum will update in real time as you change the averaging size.

