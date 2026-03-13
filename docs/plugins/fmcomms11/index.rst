.. _fmcomms11_index:

FMCOMMS11
========================================

Overview
--------

The **FMCOMMS11** plugin for Scopy enables control and configuration of the
AD-FMCOMMS11-EBZ evaluation board. It provides a unified interface to configure
the high-speed RF DAC (AD9162), high-speed ADC (AD9625), input attenuator
(HMC1119), and output variable gain amplifier (ADL5240) through the IIO
framework.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The FMCOMMS11 tool open in Scopy showing the full interface with a connected AD-FMCOMMS11-EBZ device

Features
--------

- **ADC Control:** Configure test mode and input scale for the AD9625
  high-speed ADC, with live sampling frequency readback.
- **DAC Control:** Set the NCO frequency and enable the FIR85 digital filter
  for the AD9162 RF DAC, with live sampling frequency readback.
- **Input Attenuation:** Adjust the HMC1119 step attenuator gain from 0 to
  −31.75 dB in 0.25 dB steps.
- **Output VGA:** Adjust the ADL5240 variable gain amplifier from −11.5 to
  +20 dB in 0.5 dB steps.
- **DDS Support:** Direct digital synthesis for the DAC is handled by the
  generic DAC plugin working alongside this plugin.

Supported Devices
-------------------

- **AD-FMCOMMS11-EBZ**

  - AD9162 — 16-bit, 12 GSPS RF DAC
  - AD9625 — 12-bit, 2.5 GSPS ADC
  - HMC1119 — digital step attenuator (input)
  - ADL5240 — variable gain amplifier (output)

Getting Started
---------------

**Prerequisites**

- AD-FMCOMMS11-EBZ evaluation board with a compatible carrier (e.g., ZC706)
- Linux IIO drivers loaded for all four required devices
- Scopy version 2.2.0 or later

**Using the Plugin**

Connect the AD-FMCOMMS11-EBZ to your system and open Scopy. The FMCOMMS11
plugin will be detected automatically when all four IIO devices
(``axi-ad9625-hpc``, ``axi-ad9162-hpc``, ``hmc1119``, ``adl5240``) are present
on the IIO context. Select the FMCOMMS11 tool from the device tool list to
open the control panel. Changes to any control are written to the hardware
immediately.

.. toctree::
   :maxdepth: 2

   fmcomms11
