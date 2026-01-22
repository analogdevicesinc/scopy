Plugins
================================================================================

Scopy plugins act as intermediaries, allowing users to control and interact with devices in a way
that's intuitive and easy to understand.

* **Generic** plugins offer basic commands and actions that work across a wide range of devices.
* **Device-specific** plugins are tailored to a particular device, enabling users to access its
  unique features and functions.

General IIO plugins

* :doc:`ADC plugin <adc/adc>`

* :doc:`DAC plugin <dac/dac>`

* :doc:`Datalogger <datalogger/datalogger>`

* :doc:`Debugger <debugger/index>`

* :doc:`Register Map <registermap/registermap>`

* :doc:`JESD Status <jesdstatus/index>`

* :doc:`Scripting Tool <scripting/scripting>`


Application Specific plugins

* ADALM2000 plugins :

    * :doc:`Oscilloscope <m2k/oscilloscope>`
    * :doc:`Spectrum Analyzer <m2k/spectrum_analyzer>`
    * :doc:`Network Analyzer <m2k/network_analyzer>`
    * :doc:`Signal Generator <m2k/signal_generator>`
    * :doc:`Logic Analyzer <m2k/logic_analyzer>`
    * :doc:`Pattern Generator <m2k/pattern_generator>`
    * :doc:`Digital IO <m2k/digitalio>`
    * :doc:`Voltmeter <m2k/voltmeter>`
    * :doc:`Power Supply <m2k/power_supply>`


* AD-SWIOT1L-SL

    * :doc:`Configuration <swiot1l/swiot_config>`
    * :doc:`AD74413R <swiot1l/ad74413r>`
    * :doc:`MAX14906 <swiot1l/max14906>`
    * :doc:`Faults <swiot1l/faults>`

* PQM
    * :doc:`RMS <pqm/rms>`
    * :doc:`Harmonics <pqm/harmonics>`
    * :doc:`Waveform <pqm/waveform>`
    * :doc:`System settings <pqm/settings>`

* AD936X
    * :doc:`AD936x <ad936x/index>`
    * :doc:`AD936X Advanced <ad936x/ad936x_advanced>`

* AD9084
    * :doc:`AD9084 <ad9084/ad9084>`

* ADRV9002
    * :doc:`ADRV9002 (Jupiter) Plugin <adrv9002/adrv9002>`

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

