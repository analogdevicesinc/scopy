.. _cn0511_index:

CN0511
========================================

Overview
--------

The **CN0511** plugin for Scopy provides control of the CN0511 wideband synthesizer
circuit. It allows users to configure the output frequency and amplitude of the AD9166
DAC, and to enable or disable the on-board amplifier stage, all from within Scopy's
interface.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The CN0511 tool open in Scopy showing the full interface with Single Tone Mode and DAC Amplifier sections visible

Features
--------

- **Single Tone Mode:** Configure the output frequency (0–10000 MHz) and amplitude (-91 to 0 dBm) of the AD9166 DAC.
- **DAC Amplifier Control:** Enable or disable the AD9166-AMP output amplifier stage.
- **Calibration Apply:** Write frequency, amplitude, and IOFS coefficients to the device in a single action.
- **Device Readback:** Refresh button reads the current NCO frequency back from the hardware.

Supported Devices
-------------------

- **CN0511** (Wideband Synthesizer with Integrated VCO, based on AD9166 DAC and AD9166-AMP)

Getting Started
---------------

**Prerequisites**

- A CN0511 board connected and accessible via a supported IIO context (USB, network, or local)
- Scopy version 2.1.0 or later

**Using the Plugin**

Connect to the device from Scopy's connection dialog. The CN0511 plugin will be loaded
automatically when both the ``ad9166`` DAC and ``ad9166-amp`` amplifier IIO devices are
detected. Once connected, the CN0511 tool will appear in the tool list. Set the desired
output frequency and amplitude, then click **Enter** to apply the calibration. Use the
**Refresh** button in the top-right corner to read back the current frequency from the device.

.. toctree::
   :maxdepth: 2

   cn0511
