.. _cn0540_index:

CN0540
========================================

Overview
--------

The **CN0540** plugin for Scopy enables control and monitoring of the CN0540
precision measurement system. The plugin provides a single-page interface with
collapsible sections for power control, ADC driver configuration, sensor
calibration, and analog voltage monitoring.

.. AN_IMAGE_WILL_BE_HERE
   Screenshot: The CN0540 tool open in Scopy showing the full interface with all
   four collapsible sections visible and a connected device

Features
--------

- **Power Control:** Manage device power state and GPIO-driven peripherals
  including fault detection, shutdown control, and constant current LED output.
- **ADC Driver Settings:** Configure the Fully-Differential Amplifier (FDA)
  enable state and power mode.
- **Sensor Calibration:** Read ADC input voltage, set the DAC shift voltage,
  compute sensor voltage, and run an automated iterative calibration routine.
- **Voltage Monitor:** Display six analog pin voltages updated automatically
  every second (available when an xadc or ltc2308 device is present).

Supported Devices
-------------------

- **CN0540** Precision Measurement System

  - Requires IIO devices: ``ad7768-1`` (ADC), ``ltc2606`` (DAC),
    ``one-bit-adc-dac`` (GPIO)
  - Optional: ``xadc`` or ``ltc2308`` (Voltage Monitor section)

Getting Started
---------------

**Prerequisites**

- CN0540 hardware connected and powered
- IIO kernel drivers loaded for ``ad7768-1``, ``ltc2606``, and
  ``one-bit-adc-dac``
- Scopy 2.0 or later

**Using the Plugin**

Connect the CN0540 board and open Scopy. The plugin appears in the tool menu
as **CN0540**. Use the **Refresh** button in the top bar to re-read all
hardware values. Each collapsible section can be expanded or collapsed
independently. Changes to checkboxes and voltage fields are written to the
hardware immediately.

.. toctree::
   :maxdepth: 2

   cn0540
