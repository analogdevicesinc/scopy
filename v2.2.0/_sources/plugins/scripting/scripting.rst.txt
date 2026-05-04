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
--------------------

A list of supported commands by the Scopy tool.

General
~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~

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

IIO Explorer (Debugger) API
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. rubric:: Tab Control

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.setCurrentTab(index)
     - 0-2
     - Set the current main tab (0=IIO Attributes, 1=Log, 2=Code Generator).
   * - iioExplorer.getCurrentTab()
     - -
     - Get the current main tab index.
   * - iioExplorer.showIIOAttributesTab()
     - -
     - Switch to the IIO Attributes tab.
   * - iioExplorer.showLogTab()
     - -
     - Switch to the Log tab.
   * - iioExplorer.showCodeGeneratorTab()
     - -
     - Switch to the Code Generator tab.

.. rubric:: Tree Navigation

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.selectItemByPath(path)
     - Path string
     - Select a tree item by path (e.g., "context0/ad9361-phy/voltage0/hardwaregain").
   * - iioExplorer.getSelectedItemPath()
     - -
     - Get the path of the currently selected item.
   * - iioExplorer.getSelectedItemName()
     - -
     - Get the name of the currently selected item.
   * - iioExplorer.getSelectedItemType()
     - -
     - Get the type of the currently selected item.

.. rubric:: Tree Expand/Collapse

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.expandItem(path)
     - Path string
     - Expand a tree item by path.
   * - iioExplorer.collapseItem(path)
     - Path string
     - Collapse a tree item by path.
   * - iioExplorer.isItemExpanded(path)
     - Path string
     - Check if a tree item is expanded.
   * - iioExplorer.getChildCount(path)
     - Path string
     - Get the number of children for a tree item.
   * - iioExplorer.getChildNames(path)
     - Path string
     - Get list of child names for a tree item.

.. rubric:: Watchlist

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.addCurrentItemToWatchlist()
     - -
     - Add the currently selected item to the watchlist.
   * - iioExplorer.addItemToWatchlistByPath(path)
     - Path string
     - Add a tree item to the watchlist by path.
   * - iioExplorer.removeItemFromWatchlistByPath(path)
     - Path string
     - Remove a tree item from the watchlist by path.
   * - iioExplorer.getWatchlistPaths()
     - -
     - Get list of all watchlist item paths.
   * - iioExplorer.clearWatchlist()
     - -
     - Remove all items from the watchlist.
   * - iioExplorer.refreshWatchlist()
     - -
     - Refresh all watchlist entries.
   * - iioExplorer.writeWatchlistAttributeValue(path, value)
     - Path, Value
     - Write a value to a watchlist attribute.

.. rubric:: Attribute Values

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.readAttributeValue(path)
     - Path string
     - Read the value of an attribute by path.
   * - iioExplorer.writeAttributeValue(path, value)
     - Path, Value
     - Write a value to an attribute by path.
   * - iioExplorer.triggerRead()
     - -
     - Trigger a read on all child items of the selected item.

.. rubric:: Code Generator

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.getGeneratedCode()
     - -
     - Get the generated code content.
   * - iioExplorer.saveGeneratedCode(filePath)
     - File path
     - Save the generated code to a file.

.. rubric:: Log

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.getLogContent()
     - -
     - Get the log content.
   * - iioExplorer.clearLog()
     - -
     - Clear the log.

.. rubric:: Search/Filter

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.setSearchText(text)
     - Search string
     - Set the search/filter text.
   * - iioExplorer.getSearchText()
     - -
     - Get the current search text.
   * - iioExplorer.getSearchPlaceholderText()
     - -
     - Get the search placeholder text.
   * - iioExplorer.getVisibleItemCount()
     - -
     - Get the number of visible items after filtering.

.. rubric:: Details View

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - iioExplorer.setDetailsViewTab(index)
     - 0-2
     - Set the details view tab (0=GUI View, 1=IIO View, 2=Generated Code).
   * - iioExplorer.getDetailsViewTab()
     - -
     - Get the current details view tab index.

PQM API
~~~~~~~~~~~

.. rubric:: Tool Selection

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.getTools()
     - -
     - Get list of available tool names.

.. rubric:: RMS Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.isRmsRunning()
     - -
     - Check if RMS instrument is running.
   * - pqm.setRmsRunning(running)
     - true/false
     - Start or stop the RMS instrument.
   * - pqm.rmsSingleShot()
     - -
     - Trigger a single shot acquisition.
   * - pqm.isRmsLoggingEnabled()
     - -
     - Check if RMS logging is enabled.
   * - pqm.setRmsLoggingEnabled(enabled)
     - true/false
     - Enable or disable RMS logging.
   * - pqm.getRmsLogPath()
     - -
     - Get the RMS log file path.
   * - pqm.setRmsLogPath(path)
     - File path
     - Set the RMS log file path.
   * - pqm.resetRmsPqEvents()
     - -
     - Reset RMS PQ events.
   * - pqm.isRmsPqEvents()
     - -
     - Check if RMS PQ events are active.

.. rubric:: Harmonics Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.isHarmonicsRunning()
     - -
     - Check if Harmonics instrument is running.
   * - pqm.setHarmonicsRunning(running)
     - true/false
     - Start or stop the Harmonics instrument.
   * - pqm.harmonicsSingleShot()
     - -
     - Trigger a single shot acquisition.
   * - pqm.getHarmonicsActiveChannel()
     - -
     - Get the active harmonics channel name.
   * - pqm.setHarmonicsActiveChannel(channel)
     - Channel name
     - Set the active harmonics channel.
   * - pqm.getHarmonicsType()
     - -
     - Get the current harmonics type.
   * - pqm.setHarmonicsType(type)
     - "harmonics", "inter_harmonics"
     - Set the harmonics type.
   * - pqm.isHarmonicsLoggingEnabled()
     - -
     - Check if Harmonics logging is enabled.
   * - pqm.setHarmonicsLoggingEnabled(enabled)
     - true/false
     - Enable or disable Harmonics logging.
   * - pqm.getHarmonicsLogPath()
     - -
     - Get the Harmonics log file path.
   * - pqm.setHarmonicsLogPath(path)
     - File path
     - Set the Harmonics log file path.
   * - pqm.resetHarmonicsPqEvents()
     - -
     - Reset Harmonics PQ events.
   * - pqm.isHarmonicsPqEvents()
     - -
     - Check if Harmonics PQ events are active.

.. rubric:: Waveform Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.isWaveformRunning()
     - -
     - Check if Waveform instrument is running.
   * - pqm.setWaveformRunning(running)
     - true/false
     - Start or stop the Waveform instrument.
   * - pqm.waveformSingleShot()
     - -
     - Trigger a single shot acquisition.
   * - pqm.getWaveformTimespan()
     - -
     - Get the current timespan value.
   * - pqm.setWaveformTimespan(value)
     - Seconds
     - Set the timespan value.
   * - pqm.isWaveformRollingMode()
     - -
     - Check if rolling mode is enabled.
   * - pqm.setWaveformRollingMode(enabled)
     - true/false
     - Enable or disable rolling mode.
   * - pqm.getWaveformTriggeredBy()
     - -
     - Get the trigger channel name.
   * - pqm.setWaveformTriggeredBy(channel)
     - Channel name
     - Set the trigger channel.
   * - pqm.isWaveformLoggingEnabled()
     - -
     - Check if Waveform logging is enabled.
   * - pqm.setWaveformLoggingEnabled(enabled)
     - true/false
     - Enable or disable Waveform logging.
   * - pqm.getWaveformLogPath()
     - -
     - Get the Waveform log file path.
   * - pqm.setWaveformLogPath(path)
     - File path
     - Set the Waveform log file path.

.. rubric:: Settings Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.settingsRead()
     - -
     - Read settings from device.
   * - pqm.settingsWrite()
     - -
     - Write settings to device.
   * - pqm.getSettingsAttributeValue(attrName)
     - Attribute name
     - Get the value of a settings attribute.
   * - pqm.setSettingsAttributeValue(attrName, value)
     - Attribute name, Value
     - Set the value of a settings attribute.

.. rubric:: Utility

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - pqm.triggerPqEvent(enable)
     - true/false
     - Trigger a PQ event (for testing).
   * - pqm.getLogFilePath()
     - -
     - Get the actual log file path.

SWIOT API
~~~~~~~~~~~

.. rubric:: Tool Selection

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - swiot.getTools()
     - -
     - Get list of available tool names.
   * - swiot.isRuntimeMode()
     - -
     - Check if device is in runtime mode.
   * - swiot.switchToConfigMode()
     - -
     - Switch from runtime mode to config mode.

.. rubric:: Config Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - swiot.getConfigChannelCount()
     - -
     - Get number of config channels.
   * - swiot.applyConfig()
     - -
     - Apply the current configuration.
   * - swiot.setChannelEnabled(channelId, enabled)
     - Channel ID, true/false
     - Enable or disable a config channel.
   * - swiot.isChannelEnabled(channelId)
     - Channel ID
     - Check if a config channel is enabled.
   * - swiot.setChannelDevice(channelId, device)
     - Channel ID, Device name
     - Set the device for a config channel.
   * - swiot.getChannelDevice(channelId)
     - Channel ID
     - Get the device name of a config channel.
   * - swiot.getAvailableDevices(channelId)
     - Channel ID
     - Get list of available devices for a channel.
   * - swiot.setChannelFunction(channelId, function)
     - Channel ID, Function name
     - Set the function for a config channel.
   * - swiot.getChannelFunction(channelId)
     - Channel ID
     - Get the function name of a config channel.
   * - swiot.getAvailableFunctions(channelId)
     - Channel ID
     - Get list of available functions for a channel.

.. rubric:: AD74413R Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - swiot.isAdRunning()
     - -
     - Check if AD74413R instrument is running.
   * - swiot.setAdRunning(running)
     - true/false
     - Start or stop the AD74413R instrument.
   * - swiot.adSingleShot()
     - -
     - Trigger a single shot acquisition.
   * - swiot.getAdTimespan()
     - -
     - Get the current timespan value.
   * - swiot.setAdTimespan(timespan)
     - Seconds
     - Set the timespan value.
   * - swiot.setAdPlotLabelsEnabled(enabled)
     - true/false
     - Enable or disable plot labels.
   * - swiot.isAdPlotLabelsEnabled()
     - -
     - Check if plot labels are enabled.
   * - swiot.setAdMeasurementsEnabled(enabled)
     - true/false
     - Enable or disable measurements.
   * - swiot.isAdMeasurementsEnabled()
     - -
     - Check if measurements are enabled.
   * - swiot.setAdChannelEnabled(channelId, enabled)
     - Channel ID, true/false
     - Enable or disable a channel.
   * - swiot.isAdChannelEnabled(channelId)
     - Channel ID
     - Check if a channel is enabled.
   * - swiot.setAdChannelSamplingFrequency(channelId, frequency)
     - Channel ID, Frequency
     - Set the sampling frequency for a channel.
   * - swiot.getAdChannelSamplingFrequency(channelId)
     - Channel ID
     - Get the sampling frequency of a channel.
   * - swiot.setAdChannelYMin(channelId, value)
     - Channel ID, Value
     - Set the Y axis minimum for a channel.
   * - swiot.setAdChannelYMax(channelId, value)
     - Channel ID, Value
     - Set the Y axis maximum for a channel.
   * - swiot.getAdChannelYMin(channelId)
     - Channel ID
     - Get the Y axis minimum of a channel.
   * - swiot.getAdChannelYMax(channelId)
     - Channel ID
     - Get the Y axis maximum of a channel.
   * - swiot.getAdChannelCount()
     - -
     - Get the number of AD74413R channels.
   * - swiot.getAdChannelNames()
     - -
     - Get list of AD74413R channel names.
   * - swiot.getAdSampleRate()
     - -
     - Get the current sample rate.

.. rubric:: MAX14906 Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - swiot.isMaxRunning()
     - -
     - Check if MAX14906 instrument is running.
   * - swiot.setMaxRunning(running)
     - true/false
     - Start or stop the MAX14906 instrument.
   * - swiot.getMaxTimespan()
     - -
     - Get the current timespan value.
   * - swiot.setMaxTimespan(timespan)
     - Seconds
     - Set the timespan value.
   * - swiot.setMaxChannelOutput(channelId, value)
     - Channel ID, true/false
     - Set a channel output value.
   * - swiot.getMaxChannelOutput(channelId)
     - Channel ID
     - Get a channel output value.
   * - swiot.getMaxChannelCount()
     - -
     - Get the number of MAX14906 channels.
   * - swiot.getMaxChannelNames()
     - -
     - Get list of MAX14906 channel names.
   * - swiot.getMaxXAxisMin()
     - -
     - Get the X axis minimum value.
   * - swiot.getMaxXAxisMax()
     - -
     - Get the X axis maximum value.

.. rubric:: Faults Instrument

.. list-table::
   :header-rows: 1
   :widths: 40 30 70

   * - Command
     - Values
     - Description
   * - swiot.isFaultsRunning()
     - -
     - Check if Faults instrument is running.
   * - swiot.setFaultsRunning(running)
     - true/false
     - Start or stop the Faults instrument.
   * - swiot.faultsSingleShot()
     - -
     - Trigger a single shot fault read.
   * - swiot.resetAdFaultsStored()
     - -
     - Reset AD74413R stored faults.
   * - swiot.resetMaxFaultsStored()
     - -
     - Reset MAX14906 stored faults.
   * - swiot.clearAdFaultsSelection()
     - -
     - Clear AD74413R fault bit selection.
   * - swiot.clearMaxFaultsSelection()
     - -
     - Clear MAX14906 fault bit selection.
   * - swiot.getAdActiveFaultBits()
     - -
     - Get list of active AD74413R fault bit indices.
   * - swiot.getMaxActiveFaultBits()
     - -
     - Get list of active MAX14906 fault bit indices.
   * - swiot.getAdSelectedFaultBits()
     - -
     - Get list of selected AD74413R fault bit indices.
   * - swiot.getMaxSelectedFaultBits()
     - -
     - Get list of selected MAX14906 fault bit indices.
   * - swiot.getAdStoredFaultBits()
     - -
     - Get list of stored AD74413R fault bit indices.
   * - swiot.getMaxStoredFaultBits()
     - -
     - Get list of stored MAX14906 fault bit indices.
   * - swiot.selectAdFaultBit(bitIndex)
     - Bit index
     - Select an AD74413R fault bit.
   * - swiot.selectMaxFaultBit(bitIndex)
     - Bit index
     - Select a MAX14906 fault bit.
   * - swiot.isAdFaultsExplanationEnabled()
     - -
     - Check if AD74413R faults explanation is enabled.
   * - swiot.setAdFaultsExplanationEnabled(enabled)
     - true/false
     - Enable or disable AD74413R faults explanation.
   * - swiot.isMaxFaultsExplanationEnabled()
     - -
     - Check if MAX14906 faults explanation is enabled.
   * - swiot.setMaxFaultsExplanationEnabled(enabled)
     - true/false
     - Enable or disable MAX14906 faults explanation.
