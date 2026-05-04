.. _ad9084_index:

APOLLO
========================================

Overview
--------

The **AD9084** plugin for Scopy enables integration and control of the 
Apollo MxFE platform within the Scopy software environment. This plugin 
allows users to configure and interact with the AD9084 ADC/DAC.

Features
--------
- **Device Discovery:** Automatic detection of connected AD9084/AD9088 devices.
- **Signal Acquisition:** Real-time streaming of I/Q data from the device.
- **Device Configuration:** Set all the required configuration parameters.

Supported Devices
-------------------

- **AD9084** (Apollo MxFE Quad, 16-Bit, 28GSPS RF DAC and Quad, 12-Bit, 20 GSPS RF ADC)
- **AD9088** (Apollo MxFE Octal, 16-Bit, 16 GSPS RF DAC and Octal, 12-Bit, 8 GSPS RF ADC )
- **Apollo MxFE** (with AD9084/AD9088)

Getting Started
---------------

**Prerequisites**

- A supported Apollo MxFE device (AD9084/AD9088)
- Scopy version 2.1.0 or later

**Using the Plugin**

.. note::
    The plugin can be tested using the embedded Apollo Emulated device.
    This emulated device allows users to explore the plugin features 
    without needing a physical device. 
    Choose **apollo** from the Emulator dropdown in Scopy.
    Check the IIO Emulator section for more details!


.. note::
    Make sure the AD9084 Plugin is selected in the Scopy **Add device** dialog 
    after selecting the device URI.

This plugin contains a single type of instrument, which will be instantiated 
for each compatible device found in the current IIO context:

.. toctree::
   :maxdepth: 3

   ad9084


Additional Resources
---------------------

- `AD9084 Product Page <https://www.analog.com/en/products/ad9084.html>`_
- `Apollo MxFE Product Page <https://www.analog.com/en/lp/001/apollo-mxfe.html>`_
