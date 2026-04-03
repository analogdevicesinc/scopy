.. _ad9371_index:

AD9371 / AD9375
================================================================================

Overview
--------

The AD9371 and AD9375 are wideband RF transceiver integrated circuits from Analog Devices.
The Scopy plugin supports both devices through a unified interface. When an AD9375 is connected,
the plugin automatically detects it and enables the additional DPD, CLGC, and VSWR features.

Each device exposes two tools in Scopy: a basic tool for real-time configuration of the signal
chain, and an advanced tool for low-level driver initialization settings.

Features
--------

- **Real-time control:** All basic tool changes are applied immediately and read back from hardware.
- **Dual-tool interface:** Separate basic and advanced tools for everyday use and driver-level setup.
- **Automatic device detection:** The plugin detects AD9375 hardware and enables its exclusive features.
- **FPGA DDS:** Configurable test signal generation including single-tone, dual-tone, I/Q, and
  arbitrary waveform modes.
- **BIST:** Built-in self-test with tone injection, PRBS, and digital loopback.

Supported Devices
-----------------

- **AD9371** — Wideband RF transceiver (300 MHz to 6 GHz). Covered in :ref:`ad9371` and
  :ref:`ad9371_advanced`.
- **AD9375** — Superset of AD9371 with Digital Pre-Distortion (DPD), Closed-Loop Gain Control
  (CLGC), and VSWR detection. Covered in :ref:`ad9375` and :ref:`ad9375_advanced`.

Getting Started
---------------

**Prerequisites**

- An AD9371 or AD9375 evaluation board connected to the host.
- Scopy 2.3 or later.

**Connecting**

Launch Scopy and connect to the device URI (e.g., ``ip:192.168.2.1``). The plugin loads
automatically. If an AD9375 is detected, the tools appear as **ADRV9375** and **ADRV9375 Advanced**.

.. toctree::
   :maxdepth: 2

   ad9371
   ad9371_advanced
   ad9375
   ad9375_advanced
