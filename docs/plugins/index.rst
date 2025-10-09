.. _plugins:

Plugins
================================================================================

Scopy plugins act as intermediaries, allowing users to control and interact with devices in a way
that's intuitive and easy to understand.

* **Generic** plugins offer basic commands and actions that work across a wide range of devices.
* **Device-specific** plugins are tailored to a particular device, enabling users to access its
  unique features and functions.

General IIO plugins

* :ref:`ADC Plugin <adc>`

* :ref:`DAC plugin <dac>`

* :ref:`Datalogger <datalogger>`

* :ref:`Debugger <debugger>`

* :ref:`Register Map <registermap>`

* :ref:`JESD204 Status <jesd_status>`

* :ref:`Scripting tool <scripting_tool>`


Application Specific plugins

* ADALM2000 plugins :

    * :ref:`Oscilloscope <oscilloscope>`
    * :ref:`Spectrum Analyzer <spectrum_analyzer>`
    * :ref:`Network Analyzer <network_analyzer>`
    * :ref:`Signal Generator <signal_generator>`
    * :ref:`Logic Analyzer <logic_analyzer>`
    * :ref:`Pattern Generator <pattern_generator>`
    * :ref:`Digital IO <digitalio>`
    * :ref:`Voltmeter <voltmeter>`
    * :ref:`Power Supply <power_supply>`


* AD-SWIOT1L-SL

    * :ref:`Config <swiot_config>`
    * :ref:`AD74413R <ad74413r>`
    * :ref:`MAX14906 <max14906>`
    * :ref:`Faults <faults>`

* PQM
    * :ref:`RMS <pqm-rms>`
    * :ref:`Harmonics <pqm-harmonics>`
    * :ref:`Waveform <pqm-waveform>`
    * :ref:`Settings <pqm-settings>`

* AD936X
    * :ref:`AD936X <ad936x>`
    * :ref:`AD936X Advanced <ad936x_advanced>`

* AD9084
    * :ref:`AD9084 <ad9084>`

* ADRV9002
    * :ref:`ADRV9002 (Jupiter) <adrv9002>`

Contents
---------------------------------------------------------------------

.. toctree::
   :includehidden:
   :maxdepth: 3

   adc/adc
   dac/dac
   datalogger/datalogger
   debugger/index
   registermap/registermap
   m2k/index
   pqm/index
   swiot1l/index
   ad936x/index
   ad9084/index
   adrv9002/adrv9002
   jesdstatus/index
   scripting/scripting

