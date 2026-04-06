.. _daq2_index:

DAQ2
========================================

Overview
--------

The **DAQ2** plugin for Scopy enables control and monitoring of the AD-FMCDAQ2-EBZ evaluation
board, which combines the AD9680 high-speed ADC and the AD9144 high-speed DAC. The plugin allows
users to inspect sampling frequencies, configure ADC test modes for diagnostics, and refresh
device state at any time using the built-in refresh button.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The DAQ2 tool open in Scopy showing the full interface with a connected device,
   including both the ADC and DAC sections visible

Features
--------

- **ADC Monitoring:** Displays the ADC sampling frequency read directly from the hardware.
- **Test Mode Control:** Configures each ADC channel independently into one of several built-in
  test modes (e.g. PN9, PN23, checkerboard) for digital interface validation.
- **DAC Monitoring:** Displays the DAC data clock frequency read directly from the hardware.
- **Live Refresh:** A refresh button re-reads all device attributes from hardware on demand.
- **Scripting Support:** Full JS API (object name ``daq2``) for automated test control.

Supported Devices
-----------------

- **AD-FMCDAQ2-EBZ** (AD9680 ADC + AD9144 DAC)

Getting Started
---------------

**Prerequisites**

- AD-FMCDAQ2-EBZ evaluation board connected to a supported FPGA carrier
- IIO context accessible via network (IP) or USB
- Scopy 2.x or later

**Using the Plugin**

Connect Scopy to the device URI. The DAQ2 plugin is detected automatically when both the
``axi-ad9680-hpc`` and ``axi-ad9144-hpc`` IIO devices are present in the context. Once
connected, the ADC and DAC sampling frequencies are read from hardware and displayed. Use the
Test Mode drop-downs to configure the ADC channels, and press the refresh button to re-read all
values from hardware.

.. toctree::
   :maxdepth: 2

   daq2
