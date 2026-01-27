.. _rfpowermeter:

RF Power Meter
================================================================================

General Description
---------------------------------------------------------------------

The RF Power Meter plugin transforms the Scopy Data Logger into a specialized RF power measurement instrument when connected to compatible hardware. This plugin provides real-time monitoring of RF power levels, voltage measurements, and temperature readings through an intuitive seven-segment display interface.

The RF Power Meter plugin automatically detects devices with the ``powrms`` IIO device and configures the Data Logger for RF-specific measurements. It displays forward and reverse power readings in dBm, along with corrected and raw voltage measurements in millivolts.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/rfpowermeter/rfpowermeter.png
    :align: center

.. note::
   The RF Power Meter plugin requires the Data Logger plugin to be installed and enabled.
   It transforms the Data Logger interface rather than creating a standalone tool.

.. warning::
   Ensure proper RF safety precautions when working with RF power measurement equipment.
   Follow device-specific safety guidelines and power handling limitations.

Key Features
---------------------------------------------------------------------

**Power Measurements**
  * **Forward Power**: Real-time measurement of forward RF power in dBm
  * **Reverse Power**: Detection and measurement of reflected power in dBm
  * **Bidirectional Monitoring**: Simultaneous forward and reverse power tracking

**Voltage Monitoring**
  * **Corrected Voltages**: Temperature and calibration-corrected input/output voltage readings
  * **Raw Voltages**: Uncorrected voltage measurements for diagnostic purposes
  * **Precision**: Millivolt-level measurement accuracy

Hardware Requirements
---------------------------------------------------------------------

**Compatible Devices**
  The RF Power Meter plugin requires hardware that exposes a ``powrms`` Industrial I/O (IIO) device. Compatible devices include:

  * ADI RF power measurement evaluation boards
  * Custom RF power meters with IIO interface
  * Development boards with integrated power monitoring

**Software Dependencies**
  * Scopy v2.2.0 or later
  * **Data Logger Plugin**: Must be installed and enabled 
  * IIO drivers for the target hardware

**Connection Requirements**
  * USB or Ethernet connection to the RF power measurement device
  * Stable IIO context for device communication

Plugin Operation
---------------------------------------------------------------------

**Automatic Detection**
  The plugin automatically detects compatible devices when connecting to hardware with 
  a ``powrms`` IIO device. No manual configuration is required for device recognition.

**Real-time Operation**
  Once connected and configured, the plugin provides:

  * Continuous monitoring of all configured channels
  * Live updates of power and voltage readings
  * Temperature-compensated measurements
  * Frequency-adjustable operation


Error Handling and Troubleshooting
---------------------------------------------------------------------

**Common Issues**

**Plugin Not Loading**

  * **Symptom**: RF Power Meter plugin doesn't appear when connecting
  * **Cause**: Device doesn't expose ``powrms`` IIO device
  * **Solution**: Verify hardware compatibility and IIO driver installation

**Data Logger Dependency Error**

  * **Symptom**: Error message about DataLogger plugin not found
  * **Cause**: Data Logger plugin not installed or enabled
  * **Solution**: Install and enable Data Logger plugin before using RF Power Meter

**No Measurements Displayed**

  * **Symptom**: Seven-segment display shows no data
  * **Cause**: Measurement channels not enabled or IIO communication failure
  * **Solution**: Check channel enable status and device connection


For More Information
---------------------------------------------------------------------

* **Data Logger Guide**: :ref:`datalogger`
