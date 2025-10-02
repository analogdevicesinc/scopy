.. _ad936x_index:

AD936x 
========================================

Overview
--------

The **AD936x** plugins for Scopy enable integration and control of AD936x-based devices within the Scopy software environment. 
These plugins allow users to visualize, analyze, and interact with RF signals and data streams via Scopy’s intuitive interface.

Features
--------

- **Device Discovery:** Automatic detection of connected PlutoSDR or AD936x devices.
- **Signal Acquisition:** Real-time streaming of I/Q data from the device.
- **Device Configuration:** Set parameters such as center frequency, bandwidth, sample rate, and gain.

Supported Devices
-------------------

- **AD936x Family** (including AD9361, AD9364, and compatible SDR platforms)
- **ADALM-Pluto (PlutoSDR)** (based on Analog Devices AD9363)
- **AD-FMCOMMS2/3/4** (with AD9361/AD9364)
- **AD-FMCOMMS5** (with AD9361/AD9364)

Getting Started
---------------

**Prerequisites**

- A supported SDR device (PlutoSDR or AD936x-based platform)
- Properly installed device drivers (see `ADI Drivers <https://wiki.analog.com/university/tools/pluto/drivers>`_)
- Scopy version 2.1.0 or later


**Using the Plugin**

Configuration Options
---------------------

.. list-table::
   :header-rows: 1
   :widths: 20 40 40

   * - Parameter
     - Description
     - Typical Range
   * - Center Frequency
     - Tuning frequency of the SDR
     - 325 MHz – 3.8 GHz
   * - Sample Rate
     - ADC/DAC sample rate
     - 520 kHz – 61.44 MHz
   * - Bandwidth
     - RF bandwidth
     - Up to 20 MHz
   * - RX/TX Gain
     - Receiver/transmitter gain
     - -89.75 dB to 0 dB (step: 0.25 dB)

Troubleshooting
---------------

- **Device Not Detected:** Ensure drivers are installed and the device is connected via USB.
- **Signal Quality Issues:** Check antenna connections, adjust gain and frequency.


Additional Resources
--------------------

- `ADALM-Pluto Wiki <https://wiki.analog.com/university/tools/pluto>`_
- `AD936x Family Overview <https://www.analog.com/en/products/ad9361.html>`_


.. toctree::
   :maxdepth: 2

   ad936x
   ad936x_advanced
   fmcomms5
   fmcomms5_advanced