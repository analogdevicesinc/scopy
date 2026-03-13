.. _ad6676_index:

AD6676
================================================================================

Overview
--------

The **AD6676** plugin for Scopy enables configuration and control of the AD6676 wideband
IF receiver. The AD6676 is a high-performance integrated subsystem targeting wideband
receivers with an ADC frequency range of 2925–3200 MHz. The plugin provides direct
access to all key device parameters including ADC frequency, IF bandwidth, receiver gain,
shuffler control, and test pattern injection.

.. AN_IMAGE_WILL_BE_HERE

Features
--------

- **ADC Frequency Configuration:** Set the ADC sampling frequency in the 2925–3200 MHz range.
- **Dynamic Bandwidth Control:** Configure IF bandwidth and margins; valid range is automatically
  bounded relative to the current ADC frequency.
- **IF Receiver Settings:** Set intermediate frequency, view the resulting sampling rate,
  and control receiver gain and scale.
- **Shuffler Control:** Select shuffler operating mode and threshold from available options.
- **Test Mode:** Inject standard test patterns into the data path for verification.

Supported Devices
-----------------

- **AD6676** (IIO device: ``axi-ad6676-hpc``)

Getting Started
---------------

**Prerequisites**

- AD6676 hardware connected to the host and accessible as an IIO context.
- Scopy version 2.2 or later.

**Using the Plugin**

Connect to your device in Scopy. The AD6676 plugin is automatically detected when an
``axi-ad6676-hpc`` IIO device is present in the context. All settings are written to
hardware immediately on change. Use the **Refresh** button to re-read the current
hardware state at any time.

.. toctree::
   :maxdepth: 2

   ad6676
