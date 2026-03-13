.. _fmcomms11:

FMCOMMS11
================================================================================

Description
---------------------------------

The FMCOMMS11 plugin works with Scopy to provide control of the AD-FMCOMMS11-EBZ
high-speed RF evaluation board. It exposes controls for the AD9625 ADC, AD9162
DAC, HMC1119 input step attenuator, and ADL5240 output variable gain amplifier.
Changing any field immediately writes the value to the hardware via the IIO
framework and reads it back to confirm the applied setting.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: Full FMCOMMS11 view showing all four sections (ADC, Input Attenuator, DAC, Output VGA) with a connected device

The FMCOMMS11 view is divided into four sections:

  - **ADC**
  - **Input Attenuator**
  - **DAC**
  - **Output VGA**

ADC
---------------------------------

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The ADC section showing the Sampling Frequency readout, Scale combo, and Test Mode combo with a connected AD-FMCOMMS11-EBZ

  - **Sampling Frequency (MHz):** Displays the current ADC sampling frequency
    of the AD9625 in MHz. This field is read-only; the value is read directly
    from the ``sampling_frequency`` IIO channel attribute of the ``axi-ad9625-hpc``
    device and cannot be changed from this panel.
  - **Scale:** Selects the input voltage scale for ADC channel 0. Available
    options are populated from the ``scale_available`` IIO attribute of the
    ``axi-ad9625-hpc`` device.
  - **Test Mode:** Selects the ADC test pattern mode for channel 0. Available
    options are populated from the ``test_mode_available`` IIO attribute of the
    ``axi-ad9625-hpc`` device. Use the normal operating mode during signal
    acquisition; test modes are used for characterisation and bring-up.

Input Attenuator
---------------------------------

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The Input Attenuator section showing the Hardware Gain spin control for the HMC1119 step attenuator

  - **Hardware Gain (dB):** Sets the attenuation applied by the HMC1119 digital
    step attenuator on the signal input path. The range is −31.75 dB to 0 dB
    in steps of 0.25 dB. A value of 0 dB means no attenuation; more negative
    values increase attenuation. This maps to the ``hardwaregain`` IIO attribute
    on the ``hmc1119`` device.

DAC
---------------------------------

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The DAC section showing the Sampling Frequency readout, NCO Frequency spin, and FIR85 Enable checkbox with a connected AD-FMCOMMS11-EBZ

  - **Sampling Frequency (MHz):** Displays the current DAC sampling frequency
    of the AD9162 in MHz. This field is read-only; the value is read from the
    ``sampling_frequency`` IIO channel attribute of the ``axi-ad9162-hpc`` device
    and cannot be changed from this panel.
  - **NCO Frequency (MHz):** Sets the numerically controlled oscillator (NCO)
    frequency for the AD9162 DAC. The range is 0 to 6000 MHz. This maps to the
    ``frequency_nco`` IIO attribute on the ``axi-ad9162-hpc`` device (channel
    ``altvoltage4`` if present, otherwise ``altvoltage2``).
  - **FIR85 Enable:** Enables or disables the FIR85 digital interpolation filter
    in the AD9162 DAC signal path. This maps to the ``fir85_enable`` IIO attribute
    on the ``axi-ad9162-hpc`` device (channel ``voltage0`` output).

Output VGA
---------------------------------

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The Output VGA section showing the Hardware Gain spin control for the ADL5240 variable gain amplifier

  - **Hardware Gain (dB):** Sets the gain of the ADL5240 variable gain amplifier
    on the signal output path. The range is −11.5 dB to +20 dB in steps of
    0.5 dB. This maps to the ``hardwaregain`` IIO attribute on the ``adl5240``
    device.

DDS
---------------------------------

The direct digital synthesis (DDS) section of the AD9162 DAC — including tone
generation, buffer playback, and TX channel enable — is managed by the generic
DAC plugin in Scopy. That plugin handles the ``axi-ad9162-hpc`` DDS tone
frequency, scale, and phase attributes, as well as arbitrary waveform buffer
loading. Refer to the DAC plugin documentation for details.
