.. _scripting_tool:

Scripting Tool
==============

The Scopy Scripting Tool allows users to automate and extend Scopy’s functionality using JavaScript. Scripts can interact with devices, automate measurements, and control Scopy’s UI and instruments.

Features
--------

- **Script Editor:** Write and edit JavaScript scripts directly in Scopy.
- **Load/Save:** Open and save scripts to files.
- **Run:** Execute scripts (stopping a running script is not supported).
- **Console Output:** View script output and errors in a dedicated console.
- **Command Line Input:** Enter and execute single-line commands interactively.

How Script Execution Works
--------------------------

- Scripts are executed using Qt’s built-in `QJSEngine` (ECMAScript/JavaScript engine). See the official documentation: https://doc.qt.io/qt-5/qjsengine.html
- The scripting tool provides a code editor, console, and run control.
- Scripts can be run from the editor or loaded from file.

Scripting Examples
--------------------------

1. Connect to a device and iterate through its tools

.. code-block:: javascript

    // Find and connect to the first available device
    let devices = scopy.getDevicesName();
    if (devices.length !== 0) {
        let devID = scopy.addDevice(devices[0], "");
        let connected = scopy.connectDevice(devID);
        if (connected) {
            let tools = scopy.getTools();
            for (let i = 0; i < tools.length; i++) {
                // Tool iteration
            }
            scopy.disconnectDevice(devID);
        }
    }

2. Connect to a device, switch to regmap, and read register 0x00

.. code-block:: javascript

    // Find and connect to the first available device, switch to RegMap, and read register 0x00
    let devices = scopy.getDevicesName();
    if (devices.length !== 0) {
        let devID = scopy.addDevice(devices[0], "");
        let connected = scopy.connectDevice(devID);
        if (connected) {
            let switched = scopy.switchTool(devID, "RegMap");
            if (switched) {
                let value = regmap.read(0x00);
                // Register value read
            }
            scopy.disconnectDevice(devID);
        }
    }

3. With a connected device: in the register map, read the first register, change its value to 1, read again, then restore the original value (with msleep)

.. code-block:: javascript

    // Assumes device is already connected and RegMap tool is active
    let originalValue = regmap.read(0x00);

    regmap.write(0x00, 1);
    scopy.msleep(1000);

    let newValue = regmap.read(0x00);

    regmap.write(0x00, originalValue);
    scopy.msleep(1000);

    let restoredValue = regmap.read(0x00);

Command Line Scripting
----------------------

You can also run scripts from the command line:

.. code-block:: bash

    scopy --script myscript.js

Or run a list of scripts:

.. code-block:: bash

    scopy --script-list script1.js script2.js


.. warning::

    Stopping a running script is not supported.


Supported Commands
==================

A list of supported commands by the Scopy tool.

General
-------

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Command
     - Description
   * - exit()
     - Exit Scopy application.
   * - msleep(ms)
     - Delay for a specific amount of time (specified in milliseconds).
   * - printToConsole(value)
     - Print value to the console.
   * - fileIO.writeToFile(value, path)
     - Write value in a file at specified path.
   * - fileIO.appendToFile(value, path)
     - Append value in a file at specified path.
   * - fileIO.readAll(path)
     - Read content from a file at specified path.
   * - fileIO.readLine(path, lineNr)
     - Read entire line from a file at specified path.



ADALM2000: DigitalIO
-----------------------

.. list-table::
   :header-rows: 1
   :widths: 30 25 45

   * - Command
     - Values
     - Description
   * - dio.dir[idx]
     - true(out); false(in)
     - Set channel direction (idx=0-15)
   * - dio.out[idx]
     - true(high); false(low)
     - Set channel output (idx=0-15)
   * - dio.gpi[idx]
     - true(high); false(low)
     - Check pin value (high/low)
   * - dio.running
     - true(on); false(off)
     - Run DigitalIO instrument

ADALM2000: Power Supply
-----------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 25 55

   * - Command
     - Values
     - Description
   * - power.sync
     - true(track); false(independent)
     - Set Power Supplies as Independent or Tracking control.
   * - power.tracking_percent
     - 0 - 100 (%)
     - Set Power Supplies Tracking ratio.
   * - power.dac1_enabled
     - true(on); false(off)
     - Enable/Disable Positive Power Supply.
   * - power.dac2_enabled
     - true(on); false(off)
     - Enable/Disable Negative Power Supply.
   * - power.dac1_value
     - 0 - 5 (V)
     - Set Positive Power Supply output value.
   * - power.dac2_value
     - (-5) - 0 (V)
     - Set Negative Power Supply output value.

ADALM2000: Voltmeter
---------------------

.. list-table::
   :header-rows: 1
   :widths: 30 25 55

   * - Command
     - Values
     - Description
   * - dmm.mode_ac_low_ch1
     - true(on); false(off)
     - Set Channel 1 to AC mode (20Hz-800Hz)
   * - dmm.mode_ac_low_ch2
     - true(on); false(off)
     - Set Channel 2 to AC mode (20Hz-800Hz)
   * - dmm.mode_ac_high_ch1
     - true(on); false(off)
     - Set Channel 1 to AC mode (800Hz-40kHz)
   * - dmm.mode_ac_high_ch2
     - true(on); false(off)
     - Set Channel 2 to AC mode (800Hz-40kHz)
   * - dmm.histogram_ch1
     - true(on); false(off)
     - Enable/Disable Channel 1 History plot.
   * - dmm.histogram_ch2
     - true(on); false(off)
     - Enable/Disable Channel 2 History plot.
   * - dmm.history_ch1_size_idx
     - 0 - 2
     - Channel 1 History plot size (1s, 10s, 60s).
   * - dmm.history_ch2_size_idx
     - 0 - 2
     - Channel 2 History plot size (1s, 10s, 60s).
   * - dmm.value_ch1
     - Volts
     - Read Channel 1 value.
   * - dmm.value_ch2
     - Volts
     - Read Channel 2 value.
   * - dmm.peak_hold_en
     - true(on); false(off)
     - Enable/Disable Peak Hold.
   * - dmm.data_logging_en
     - true(on); false(off)
     - Enable/Disable Data Logging.
   * - dmm.data_logging_timer
     - 0 - 3600 (s)
     - Set Data Logging Time.
   * - dmm.data_logging_append
     - true(append); false(overwrite)
     - Overwrite/Append data logging to file.
   * - dmm.running
     - true(on); false(off)
     - Run Voltmeter instrument

ADALM2000: Signal Generator
----------------------------

.. list-table::
   :header-rows: 1
   :widths: 35 30 65

   * - Command
     - Values
     - Description
   * - siggen.mode[idx]
     - 0 - 3
     - Set Channel 1/2 (idx=0/1) Operation mode (Constant, Waveform, Buffer, Math).
   * - siggen.constant_volts[idx]
     - (-5) - 5 (V)
     - Set Channel 1/2 (idx=0/1) voltage in Constant Mode.
   * - siggen.waveform_type[idx]
     - 0 - 5
     - Set Channel 1/2 (idx=0/1) waveform type in Waveform Mode (Sine, Square, Triangle, Trapezoidal, Rising Ramp Sawtooth, Falling Ramp Sawtooth).
   * - siggen.waveform_amplitude[idx]
     - 0.000001 - 5 (V)
     - Set Channel 1/2 (idx=0/1) waveform amplitude in Waveform Mode.
   * - siggen.waveform_frequency[idx]
     - 0.001 - 37500000 (Hz)
     - Set Channel 1/2 (idx=0/1) waveform frequency in Waveform Mode.
   * - siggen.waveform_offset[idx]
     - (-5) - 5 (V)
     - Set Channel 1/2 (idx=0/1) waveform offset in Waveform Mode.
   * - siggen.waveform_phase[idx]
 
     - 0 - 359 (deg)
     - Set Channel 1/2 (idx=0/1) waveform phase in Waveform Mode.
   * - siggen.waveform_duty[idx]
 
     - 0 - 100 (%)
     - Set Channel 1/2 (idx=0/1) waveform duty in Waveform Mode (Square).
   * - siggen.waveform_rise[idx]
 
     - 0.00000001 - 10 (s)
     - Set Channel 1/2 (idx=0/1) waveform rise time in Waveform Mode (Trapezoidal).
   * - siggen.waveform_fall[idx]
 
     - 0.00000001 - 10 (s)
     - Set Channel 1/2 (idx=0/1) waveform fall time in Waveform Mode (Trapezoidal).
   * - siggen.waveform_holdhigh[idx]
 
     - 0.00000001 - 10 (s)
     - Set Channel 1/2 (idx=0/1) waveform high time in Waveform Mode (Trapezoidal).
   * - siggen.waveform_holdlow[idx]
 
     - 0.00000001 - 10 (s)
     - Set Channel 1/2 (idx=0/1) waveform low time in Waveform Mode (Trapezoidal).
   * - siggen.noise_type[idx]
 
     - 0(None); 200 - 203
     - Set Channel 1/2 (idx=0/1) Noise Type (None, Uniform, Gaussian, Laplacian, Impulse).
   * - siggen.noise_amplitude[idx]
 
     - 0.000001 - 10 (V)
     - Set Channel 1/2 (idx=0/1) Noise Amplitude.
   * - siggen.math_function[idx]
 
     - String
     - Set Channel 1/2 (idx=0/1) Math function.
   * - siggen.math_frequency[idx]
 
     - 0.001 - 37500000 (Hz)
     - Set Channel 1/2 (idx=0/1) Math frequency.
   * - siggen.buffer_file_path[idx]
 
     - String
     - Set Channel 1/2 (idx=0/1) Buffer file Path.
   * - siggen.buffer_amplitude[idx]
 
     - 0.000000001 - 10 (V)
     - Set Channel 1/2 (idx=0/1) Buffer amplitude.
   * - siggen.buffer_offset[idx]
 
     - (-5) - 5 (V)
     - Set Channel 1/2 (idx=0/1) Buffer offset.
   * - siggen.buffer_sample_rate[idx]
 
     - 0.1 - 37500000 (V)
     - Set Channel 1/2 (idx=0/1) Buffer sample rate.
   * - siggen.buffer_phase[idx]
 
     - 0 - 359 (deg)
     - Set Channel 1/2 (idx=0/1) Buffer phase.
   * - siggen.enabled[idx]
 
     - true(on); false(off)
     - Enable/Disable Channel 1/2 (idx=0/1)
   * - siggen.running
 
     - true(on); false(off)
     - Run Signal Generator instrument.

ADALM2000: Network Analyzer
--------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 25 55

   * - Command
     - Values
     - Description
   * - network.min_freq
     - 1 - 30000000 (Hz)
     - Set Sweep Minimum Frequency.
   * - network.max_freq
     - 1 - 30000000 (Hz)
     - Set Sweep Maximum Frequency.
   * - network.samples_count
     - 10 - 1000 (samples)
     - Set Sweep Samples Count.
   * - network.amplitude
     - 0.000001 - 10 (V)
     - Set Waveform Amplitude.
   * - network.offset
     - (-5) - 5 (V)
     - Set Waveform Offset.
   * - network.min_mag
     - (-120) - 120 (dB)
     - Set Minimum Magnitude displayed.
   * - network.max_mag
     - (-120) - 120 (dB)
     - Set Maximum Magnitude displayed.
   * - network.min_phase
     - (-180) - 180 (deg)
     - Set Minimum Phase displayed.
   * - network.max_phase
     - (-180) - 180 (deg)
     - Set Maximum Phase displayed.
   * - network.log_freq
     - true(log)/false(lin)
     - Set Logarithmic/Linear plot scale display.
   * - network.ref_channel
     - 1; 2
     - Set Channel 1/2 as reference channel.
   * - network.line_thickness
     - 0-9
     - Set plot line thickness (0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5)
   * - network.cursors
     - true(on); false(off)
     - Enable/Disable Plot Cursors.
   * - network.cursors_position
     - 0 - 3
     - Change Cursors Table position on plot.
   * - network.cursors_transparency
     - 0 - 100 (%)
     - Change Cursors Table transparency on plot.
   * - network.running
     - true(on); false(off)
     - Run Network Analyzer instrument.

ADALM2000: Oscilloscope
-------------------------

.. list-table::
   :header-rows: 1
   :widths: 35 30 65

   * - Command
     - Values
     - Description
   * - osc.channels[idx].enabled
     - true(on); false(off)
     - Enable/Disable Channel 1/2 (idx=0/1)
   * - osc.channels[idx].volts_per_div
     - 0.001 - 10 (V)
     - Set Channel 1/2 (idx=0/1) Volts/Div value for the plot.
   * - osc.channels[idx].ac_coupling
     - true(on); false(off)
     - Enable/Disable Channel 1/2 (idx=0/1) AC Coupling.
   * - osc.channels[idx].period
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's period.
   * - osc.channels[idx].mean
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's mean value.
   * - osc.channels[idx].cycle_mean
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's cycle mean value.
   * - osc.channels[idx].rms
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's RMS value.
   * - osc.channels[idx].cycle_rms
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's Cycle RMS value.
   * - osc.channels[idx].ac_rms
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's AC RMS value.
   * - osc.channels[idx].area
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's Area value.
   * - osc.channels[idx].cycle_area
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's Cycle Area value.
   * - osc.channels[idx].min
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's min value.
   * - osc.channels[idx].max
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's max value.
   * - osc.channels[idx].amplitude
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's amplitude value.
   * - osc.channels[idx].peak_to_peak
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's peak-to-peak value.
   * - osc.channels[idx].low
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's low value.
   * - osc.channels[idx].high
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's high value.
   * - osc.channels[idx].middle
     - ( V )
     - Read Channel 1/2 (idx=0/1) signal's middle value.
   * - osc.channels[idx].pos_overshoot
     - (%)
     - Read Channel 1/2 (idx=0/1) signal's positive overshoot value.
   * - osc.channels[idx].neg_overshoot
     - (%)
     - Read Channel 1/2 (idx=0/1) signal's negative overshoot value.
   * - osc.channels[idx].rise
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's rise time value.
   * - osc.channels[idx].fall
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's fall time value.
   * - osc.channels[idx].pos_width
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's positive width value.
   * - osc.channels[idx].neg_width
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's negative width value.
   * - osc.channels[idx].pos_duty
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's positive duty cycle value.
   * - osc.channels[idx].neg_duty
     - ( s )
     - Read Channel 1/2 (idx=0/1) signal's negative duty cycle value.
   * - osc.time_base
     - 0.0000001 - 1 (s)
     - Set Oscilloscope time base.
   * - osc.time_position
     - (-5) - 5 (s)
     - Set Oscilloscope time position.
   * - osc.measure_all
     - true(on); false(off)
     - Enable/Disable Display All Measurements.
   * - osc.measure
     - true(on); false(off)
     - Enable/Disable Measure Feature.
   * - osc.counter
     - true(on); false(off)
     - Enable/Disable Counter feature for Measurements.
   * - osc.statistics
     - true(on); false(off)
     - Enable/Disable Statistics feature for Measurements.
   * - osc.cursors
     - true(on); false(off)
     - Enable/Disable Cursors on plot.
   * - osc.horizontal_cursors
     - true(on); false(off)
     - Enable/Disable Horizontal Cursors on plot.
   * - osc.vertical_cursors
     - true(on); false(off)
     - Enable/Disable Vertical Cursors on plot.
   * - osc.cursors_position
     - 0-3
     - Set Cursors Table position on plot.
   * - osc.cursors_transparency
     - 0-100 (%)
     - Set Cursors Table transparency on plot.
   * - osc.auto_trigger
     - true(auto); false(normal)
     - Set Trigger mode to Auto/Normal.
   * - osc.internal_trigger
     - true(on); false(off)
     - Enable/Disable Internal Trigger.
   * - osc.external_trigger
     - true(on); false(off)
     - Enable/Disable External Trigger.
   * - osc.trigger_source
     - 0; 1
     - Select Channel 1/2 (0/1) as Trigger Source.
   * - osc.trigger_level
     - (-25) - 25 (V)
     - Set Trigger Level.
   * - osc.trigger_hysteresis
     - 0 - 2.5 (V)
     - Set Trigger Hysteresis.
   * - osc.running
     - true(on); false(off)
     - Run Oscilloscope instrument.

ADALM2000: Pattern Generator
------------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 30 60

   * - Command
     - Values
     - Description
   * - pattern.running
     - true(on); false(off)
     - Run Pattern Generator instrument.

ADALM2000: Logic Analyzer
-----------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 30 60

   * - Command
     - Values
     - Description
   * - logic.running
     - true(on); false(off)
     - Run Logic Analyzer instrument.

ADALM2000: Spectrum Analyzer
-------------------------------

.. list-table::
   :header-rows: 1
   :widths: 30 30 60

   * - Command
     - Values
     - Description
   * - spectrum.running
     - true(on); false(off)
     - Run Spectrum Analyzer instrument.


RegMap API
----------

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - regmap.write(addr, val)
     - Address, Value
     - Write value to register address.
   * - regmap.writeBitField(addr, val)
     - Address, Value
     - Write value to register bitfield.
   * - regmap.getAvailableDevicesName()
     - -
     - Get list of available device names.
   * - regmap.setDevice(device)
     - Device name
     - Set the active device.
   * - regmap.search(searchParam)
     - Search string
     - Search for registers or bitfields.
   * - regmap.readInterval(startAddr, stopAddr)
     - Start, Stop address
     - Read registers in address interval.
   * - regmap.enableAutoread(enable)
     - true/false
     - Enable or disable autoread.
   * - regmap.isAutoreadEnabled()
     - -
     - Check if autoread is enabled.
   * - regmap.registerDump(filePath)
     - File path
     - Dump all register values to file.
   * - regmap.setPath(filePath)
     - File path
     - Set file path for operations.
   * - regmap.writeFromFile(filePath)
     - File path
     - Write register values from file.
   * - regmap.readRegister(addr)
     - Address
     - Read value from register address.
   * - regmap.getValueOfRegister(addr)
     - Address
     - Get value of register.
   * - regmap.getRegisterInfo(addr)
     - Address
     - Get info about register.
   * - regmap.getRegisterBitFieldsInfo(addr)
     - Address
     - Get info about register bitfields.
   * - regmap.getBitFieldInfo(addr, bitName)
     - Address, Bit name
     - Get info about a specific bitfield.

DataLogger API
--------------

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - datalogger.showAvailableMonitors()
     - -
     - Show available monitors.
   * - datalogger.showAvailableDevices()
     - -
     - Show available devices.
   * - datalogger.showMonitorsOfDevice(device)
     - Device name
     - Show monitors for a device.
   * - datalogger.enableMonitor(monitor)
     - Monitor name
     - Enable a monitor.
   * - datalogger.disableMonitor(monitor)
     - Monitor name
     - Disable a monitor.
   * - datalogger.setRunning(running)
     - true/false
     - Start or stop logging.
   * - datalogger.clearData()
     - -
     - Clear logged data.
   * - datalogger.changeTool(name)
     - Tool name
     - Change active tool.
   * - datalogger.setMinMax(enable)
     - true/false
     - Enable or disable min/max display.
   * - datalogger.changePrecision(decimals)
     - Integer
     - Change display precision.
   * - datalogger.setMinYAxis(min)
     - Value
     - Set minimum Y axis value.
   * - datalogger.setMaxYAxis(max)
     - Value
     - Set maximum Y axis value.
   * - datalogger.createTool()
     - -
     - Create a new tool.
   * - datalogger.getToolList()
     - -
     - Get list of tools.
   * - datalogger.enableMonitorOfTool(toolName, monitor)
     - Tool, Monitor
     - Enable monitor for tool.
   * - datalogger.disableMonitorOfTool(toolName, monitor)
     - Tool, Monitor
     - Disable monitor for tool.
   * - datalogger.setLogPathOfTool(toolName, path)
     - Tool, Path
     - Set log file path for tool.
   * - datalogger.logAtPathForTool(toolName, path)
     - Tool, Path
     - Log data at path for tool.
   * - datalogger.continuousLogAtPathForTool(toolName, path)
     - Tool, Path
     - Start continuous logging for tool.
   * - datalogger.stopContinuousLogForTool(toolName)
     - Tool
     - Stop continuous logging for tool.
   * - datalogger.importDataFromPathForTool(toolName, path)
     - Tool, Path
     - Import data from file for tool.
