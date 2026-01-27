.. _adrv9009_index:

ADRV9009 Plugin
========================================

Overview
--------

The **ADRV9009** plugins for Scopy enable comprehensive integration and control of ADRV9009-based transceiver systems within the Scopy software environment.
These plugins provide both basic and advanced control interfaces, allowing users to configure, calibrate, and optimize ADRV9009 transceiver performance for a wide range of RF applications.

Features
--------

- **Device Discovery:** Automatic detection of connected ADRV9009 transceiver devices
- **Multi-Device Support:** Synchronized operation across multiple ADRV9009 transceivers
- **Real-time Configuration:** Live hardware configuration with immediate feedback
- **Advanced Calibrations:** Comprehensive calibration system for optimal performance
- **Multi-Chip Sync:** Hardware-synchronized multi-device operation


Getting Started
---------------

**Prerequisites**

- A supported ADRV9009-based platform or evaluation board
- Properly installed device drivers (see `ADI Linux Drivers <https://wiki.analog.com/resources/tools-software/linux-drivers>`_)
- Scopy version 2.3.0 or later

**Using the Plugins**

The ADRV9009 plugins are automatically detected when a compatible device is connected. Choose between:

- **ADRV9009 Plugin:** Standard interface for common transceiver operations
- **ADRV9009 Advanced Plugin:** Expert-level access to advanced features and calibrations


Multi-Device Capabilities
-------------------------

**Multi-Chip Synchronization (MCS):**
- Automatic device detection and enumeration
- Hardware-synchronized operation across multiple transceivers

**Applications:**
- 3G, 4G, and 5G TDD macrocell base stations
- TDD active antenna systems
- Massive multiple input, multiple output (MIMO)
- Phased array radar
- Electronic warfare
- Military communications
- Portable test equipment

Troubleshooting
---------------

**Device Detection Issues:**
- Verify ADRV9009 device connections and power
- Check libiio installation and device enumeration
- Ensure proper FPGA bitstream loading

**Performance Issues:**
- Allow adequate device warm-up time
- Verify RF signal levels and connections
- Check for proper grounding and shielding

Additional Resources
--------------------

- `ADRV9009 Product Page <https://www.analog.com/en/products/adrv9009.html>`_
- `ADRV9009 Linux Driver Documentation <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/adrv9009>`_
- `ADRV9009 Evaluation Platform Guide <https://wiki.analog.com/resources/eval/user-guides/adrv9009>`_

.. toctree::
   :maxdepth: 2

   adrv9009
   adrv9009_advanced