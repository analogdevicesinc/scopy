.. _daq2:

DAQ2
================================================================================

The DAQ2 plugin works with Scopy. Changing any field will immediately write
changes to the DAQ2 settings to the hardware, and then read it back to make
sure the setting is valid. If you want to set something that the GUI changes
to a different number, that means the hardware (either the AD9680, the AD9144,
or the FPGA fabric) does not support that mode or precision.

If you want to re-read all current values from hardware (e.g. after a settings
change outside Scopy), click the **Refresh** button in the top-right corner of
the tool.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The full DAQ2 tool view showing the ADC section and DAC section
   with the refresh button visible in the top-right corner

ADC
---------------------------------

The ADC section controls the **AD9680** analog-to-digital converter
(IIO device ``axi-ad9680-hpc``). Both channels share the same sampling clock
but can be placed into independent test modes.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The ADC section showing the sampling frequency label and the
   Ch0 Test Mode and Ch1 Test Mode combo boxes

- **Sampling Frequency:** Displays the ADC sample rate in MHz. This value is
  read from the hardware once at connect time and is not configurable from
  this plugin.
- **Ch0 Test Mode:** Selects the built-in test mode for ADC channel 0
  (``voltage0``). Available options are read from the ``test_mode_available``
  attribute on the hardware. Common values include:

  - ``off`` — normal operation
  - ``midscale_short`` — outputs a mid-scale constant value
  - ``pos_fullscale`` — outputs positive full-scale
  - ``neg_fullscale`` — outputs negative full-scale
  - ``checkerboard`` — alternating checkerboard pattern
  - ``pn9`` — PN9 pseudo-random sequence
  - ``pn23`` — PN23 pseudo-random sequence
  - ``one_zero_toggle`` — alternating 0x0000 / 0xFFFF output
  - ``ramp`` — incrementing ramp pattern

- **Ch1 Test Mode:** Selects the built-in test mode for ADC channel 1
  (``voltage1``). Accepts the same options as Ch0 Test Mode.

DAC
---------------------------------

The DAC section monitors the **AD9144** digital-to-analog converter
(IIO device ``axi-ad9144-hpc``).

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The DAC section showing the sampling frequency label

- **Sampling Frequency:** Displays the DAC data clock in MHz. This value is
  read from the hardware once at connect time and is not configurable from
  this plugin.

.. note::
   DDS tone generation and DAC buffer output controls (One CW Tone, Two CW
   Tones, Independent I/Q Control, DAC Buffer Output) are part of the full
   FMCDAQ2 feature set but are not yet available in this Scopy release. For
   complete device documentation, see the
   `FMCDAQ2 Plugin wiki page <https://wiki.analog.com/resources/tools-software/linux-software/fmcdaq2_plugin>`_.
