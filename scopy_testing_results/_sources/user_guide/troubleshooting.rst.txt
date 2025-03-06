.. _troubleshooting:

Scopy troubleshooting guide
===========================================================

Scopy v2 after Scopy v1?
------------------------------------------------------------

If this is the first time installing Scopy v2 after using
any Scopy v1 on your system, we **strongly** recommend to
uninstall the old Scopy properly.

Doing this we make sure that the configuration files for
Scopy v2 are not in any way influenced by Scopy v1.

.. _inifiles:

Scopy v1 ini files in Scopy v2
------------------------------------------------------------
Scopy v1.x ini files have a different format from version 2.0

Scopy v1.x format (snippet):

.. code-block:: bash

   [power]
   dac1_enabled=false
   dac1_value=0
   dac2_enabled=false
   dac2_value=0
   notes=
   sync=false
   tracking_percent=100
   
   [siggen]
   autoscale=false
   buffer_amplitude\1\idx=1
   buffer_amplitude\2\idx=1
   buffer_amplitude\size=2
   buffer_file_path\1\idx=
   same info in 

Scopy v2.0 format (snippet):

.. code-block:: bash

   [M2kPlugin]
   power\dac1_enabled=false
   power\dac1_value=0
   power\dac2_enabled=false
   power\dac2_value=0
   power\notes=
   power\sync=false
   power\tracking_percent=100
   siggen\autoscale=false
   siggen\buffer_amplitude\1\idx=1
   siggen\buffer_amplitude\2\idx=1
   siggen\buffer_amplitude\size=2
   siggen\buffer_file_path\1\idx=
   siggen\buffer_file_path\2\idx=
   siggen\buffer_file_path\size=2
   siggen\buffer_offset\1\idx=0


As you can see the format of the Scopy v2.0 is different, the main key is the plugin it is part of (in this case M2kPlugin) and the rest of the format is flattened. We created a small tool to help you switch from one format to another. This is a python script found `here <https://github.com/analogdevicesinc/scopy/blob/main/tools/scopy_ini_converter.py>`__. You can run it like this.

.. code-block:: bash

   python3 ./scopy_ini_converter.py --outmode v2 --input scopy_v1_config.ini --output scopy_v2_config.ini

This script will convert ini files from version 1 to version2.

Any unexpected behavior? Send us the log!
------------------------------------------------------------

Scopy v2 brings a new logging mechanism for an easier navigation of the
debugging process. Whether starting Scopy on Linux (with **flatpak** or **appimage**),
on MacOS (from **command line**) or Windows, Scopy will display a filtered log of
operations.

The filter is configured by default to display only a few main
components. This filter can be modified at runtime, by manipulating
the responsible environment variable: QT_LOGGING_RULES.

This environment variable is part of the Qt mechanism for exposing
warning and debugging messages. Read more detailed information
in the official `Qt documentation <https://doc.qt.io/qt-5/qloggingcategory.html>`__

.. tip::

   Send us the log output of Scopy to make it easier for us to reproduce
   the issue!

How to set the environment variable:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

We can modify the debug output of Scopy even if the release has a
preconfigured filter.

The default Scopy configuration is the following:

.. code-block:: cpp

						 "*.debug=false\n"
						 "ToolStack.debug=true\n"
						 "ToolManager.debug=true\n"
						 "DeviceManager.debug=true\n"
						 "Device.debug=true\n"
						 "TestPlugin.debug=true\n"
						 "Plugin.debug=true\n"
						 "swiotConfig.debug=true\n"
						 "CyclicalTask.debug=false\n"
						 "SWIOTPlugin.debug=true\n"
						 "AD74413R.debug=true\n"
						 "ScopyTranslations.debug=true\n"
						 "GRTimeSinkComponent.debug=true\n"
						 "GRManager.debug=true\n"


Using **\*.debug=false** will suppress all the debug output of Scopy.

Now let's take some examples and check them on different Operating Systems:

.. code-block:: shell

   ## This will disable all the output and only enable logs
   ## for the ADC Plugin
   "*.debug=false;ADCPlugin.debug=true"


Linux x86_64, arm64, arm32, MacOS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to modify the Scopy logging rules, open up a terminal
and run the following:

.. code-block:: shell

   export QT_LOGGING_RULES="*.debug=false;ADCPlugin.debug=true"

After that, run Scopy (specific to each OS type) and observe
the log output changing.


Windows
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In the Scopy install location on Windows you will find 2 separate
executables: **Scopy.exe** and **Scopy-console.exe**. In order to view
the debug log of Scopy, you need to run the latter: *Scopy-console.exe*.
This will not only open the Scopy window, it will also start a command
prompt in which all the output will be visible.

In order to modify the QT_LOGGING_RULES on Windows, firstly open a
command prompt and run the following:

.. code-block:: shell

   set QT_LOGGING_RULES="*.debug=false;ADCPlugin.debug=true"

After that, you can run *Scopy-console.exe* from the **same** command
prompt.


Available configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A list of all the available Scopy class filters is:

.. list-table::
   :widths: 50
   :header-rows: 1

   * - SCOPY CLASS FILTER
   * - AnimationPushButton
   * - MapStackedWidget
   * - TimeChannelComponent
   * - TutorialOverlay
   * - Time Y-Autoscale
   * - TutorialChapter
   * - PageNavigationWidget
   * - TitleSpinBox
   * - ScopyStatusBar
   * - MenuCollapseSection
   * - iioWidget
   * - AttrDataStrategy
   * - MultiDataStrategy
   * - CmdQDeviceDataStrategy
   * - CmdQChannelDataStrategy
   * - ContextAttrDataStrategy
   * - TriggerDataStrategy
   * - DeviceDataStrategy
   * - SwitchGuiStrategy
   * - AttrGuiStrategy
   * - TimeChannelAddon
   * - GRTimePlotAddon
   * - GRTimeChannel
   * - RuntimeEnvironmentInfo
   * - CrashReport
   * - toolLauncher
   * - oscilloscope
   * - signalGenerator
   * - voltmeter
   * - powerController
   * - spectrumAnalyzer
   * - networkAnalyzer
   * - digitalIO
   * - logicAnalyzer
   * - patternGenerator
   * - calibration
   * - calibration.manual
   * - iioManager
   * - plot
   * - Benchmark
   * - DetachedToolWindow
   * - ScopyTitleManager
   * - DeviceManager
   * - ScopyPreferencesPage
   * - ToolStack
   * - EmuAddPage
   * - Scopy
   * - DeviceBrowser
   * - InfoPageStack
   * - ScopyHomeAddPage
   * - PluginManager
   * - Device
   * - ToolMenuItem
   * - Scopy_API
   * - PluginRepository
   * - ScopyTranslations
   * - ScannedIIOContextCollector
   * - IIODevice
   * - ToolMenuManager
   * - IIOTabWidget
   * - GRManager
   * - TimeSink_f
   * - CmdQPingTask
   * - CyclicalTask
   * - CommandQueue
   * - ConnectionProvider
   * - IIOScanTask
   * - GUITestPlugin
   * - BareMinimum
   * - DacPlugin
   * - DacInstrument
   * - DacDds
   * - DacBuffer
   * - DacData
   * - DacDataBuilder
   * - DacFileManager
   * - DacDataStrategy
   * - RegReadWrite
   * - oscilloscope
   * - signalGenerator
   * - voltmeter
   * - powerController
   * - spectrumAnalyzer
   * - networkAnalyzer
   * - digitalIO
   * - logicAnalyzer
   * - patternGenerator
   * - calibration
   * - calibration.manual
   * - iioManager
   * - plot
   * - REGISTERMAP
   * - RegisterMapTable
   * - RegMap_API
   * - MeasurementController
   * - ADCInstrument
   * - ADCPlugin
   * - AcqTreeNode
   * - GRFFTSinkComponent
   * - GRFFTChannelComponent
   * - GRTimeSinkComponent
   * - GRTimeChannelComponent
   * - PqmRms
   * - PQMLog
   * - PqmAqcManager
   * - PQMPlugin
   * - pqm_settings
   * - M2kDigitalIo
   * - M2kIIOManager
   * - M2kCalibrationManual
   * - M2KPowerController
   * - M2kSiggen
   * - M2kNetworkAnalyzer
   * - M2kOscilloscope
   * - M2KCalibration
   * - M2kSpectrumAnalyzer
   * - M2kDMM
   * - M2KPLUGIN
   * - Benchmark
   * - Plot
   * - SWIOTPlugin
   * - AD74413R
   * - SWIOTConfig
   * - SWIOTFaults
   * - MAX14906
   * - DebuggerTreeView
   * - DebuggerDetailsView
   * - DebuggerWatchList
   * - DebuggerIIOModel
   * - IIODebugger
   * - DEBUGGERPLUGIN
   * - Benchmark
   * - TestPluginIp
   * - TestPlugin
   * - TimeTracker
   * - DataMonitor_API
   * - DataMonitorSettings
   * - DataLoggerPlugin
   * - DataLoggerPlugin
   * - DataMonitorTool
   * - ApiObject
   * - ResourceManager
   * - VersionCache
   * - Scopy_JS
   * - Plugin
   * - StatusBarManager
   * - Preferences

If you need to regenerate this list after updating parts of Scopy, you can
use the following command:

.. code-block:: shell

   grep -InH -r --exclude="*.rst*" --exclude="*.mako" --exclude="*build*" --exclude="*html*" "Q_LOGGING_CATEGORY" . | sed 's/.*Q_LOGGING_CATEGORY([^,]*, *"\([^"]*\)").*/\1/'


Linux Wayland crash
----------------------------

If you are using Scopy on a Linux system with Wayland and you encounter a crash
when trying to open the application, you can run Scopy with XCB.

.. tip::

    flatpak run --env=QT_QPA_PLATFORM=xcb org.adi.Scopy

.. tip::

    QT_QPA_PLATFORM=xcb ./Scopy.AppImage


Device doesn't show up
-------------------------------------

Make sure you install the latest driver :

   -  `Windows
      Drivers <https://wiki.analog.com/university/tools/pluto/drivers/windows>`__
   -  `Linux
      Drivers <https://wiki.analog.com/university/tools/pluto/drivers/linux>`__
   -  `macOS
      Drivers <https://wiki.analog.com/university/tools/pluto/drivers/osx>`__

Use compliant USB cable
-------------------------------------

Some USB cables are incapable of power delivery necessary to the ADALM2000 (or
are just plain flakey). Please use the provided USB cable. Also in order to
troubleshoot this, you try using two cables, plug in a separate cable to the USB
connector for power delivery (to the USB power connector), while using the
center USB connector for data transfer. If using two cables works, it normally
means it's a cable, or voltage droop issue on the host.

Reinstall & INI files
-------------------------------------

**Scopy.ini** and **Preferences.ini** contain the user-settings that are saved
in-between sessions. Sometimes these can get messed up and can cause problems.
In order to reset them, go to: These file are located in:

-  **Windows:** C:\\Users\\<your_username>\\AppData\\Local\\ADI\\Scopy-v2  
   Sometimes this is a hidden directory, and you `unhide
   it <https://cybertext.wordpress.com/2012/05/29/cant-see-the-appdata-folder/>`__.
-  **Linux:** /home/<your_username>/.config/ADI/Scopy-v2
-  **Mac:** /Users/<your_username>/.config/analog.com

and delete (or rename) **Scopy.ini** **Scopy.ini.bak** **Preferences.ini**
**Preferences.ini.bak** The preference is to rename things with a different
suffix, so if you need to report a bug, the file still exists.

You can also reset these files to default using the **Reset button** from 
the Preferences page.

Scopy configuration can sometimes get messed up when updating as opposed to
uninstall/reinstall. If you are facing this issue, you can try uninstalling
Scopy and then reinstall.

.. warning::

   Make sure that after uninstall Scopy's folder is clear

Make sure you install Scopy in a location that doesn't require any write
privileges since Scopy needs to write to it's installation folder.

Connecting through the network interface
-----------------------------------------

If you are having trouble connecting through USB, you can always connect to the
device through the network interface. The USB connection can cause all kinds of
problems on various systems. In order to connect through the emulated network
interface follow
:ref:`the steps from user guide under Connecting to a remote device. <user_guide>`

Make sure you have the latest firmware
----------------------------------------

For ADALM2000 visit the firmware page
https://github.com/analogdevicesinc/m2k-fw/releases.

Compare the latest firmware version on this page with the one you have
installed. Follow `these
steps <https://wiki.analog.com/university/tools/m2k/common/firmware>`__ to
update the firmware.

Uninstall and reinstall the software
-------------------------------------

   -  Run a clean uninstall
   -  Delete the ini files
   -  Run a clean reinstall
   -  Restart your computer after the clean install can fix your problems.

Make sure the system doesn't limit the USB power consumption
--------------------------------------------------------------

Some laptops limit the amount of current that can be delivered via an USB port
(either native or from a docking station/usb hub). In order to workaround this
issue you can try the following:

   -  Go to Device Manager tool in Windows OS
   -  Go to Universal Serial Bus Controllers
   -  Select ASMedia USB Root hub(or which USB root hub the board is connected
      to), Right click on its properties
   -  uncheck the "Allow the computer to turn off this device to save power" in
      the Power management tab
   -  Reboot.

Run iio_info -a for board diagnostics
-------------------------------------

Download latest version of https://github.com/analogdevicesinc/libiio/releases .

Open a command prompt and run

.. note::

   iio_info -a

or

.. note::

   iio_info -u "ip:192.168.2.1"


Emulator not found
--------------------------------------------------------

If you are using any binary from the official release artifacts
and you get something similar to: *"iio-emu not found"* when
trying to use the Emulator functionality, go to **Preferences**,
use the **Reset button** to set everything to its default value
and restart Scopy.


Scopy white screen on Windows
-------------------------------------

Scopy displaying a white screen on startup (combined with hangs or crashes) is an 
issue that indicates OpenGL incompatibility with your system due to a missing or 
outdated graphics driver/DirectX/OpenGl Driver. 

  * Disable OpenGL usage in favor software rendering by setting the 
    **general_use_opengl** preference in preferences.ini to **false**. 
  * On Windows Virtual Machine, enable 3d hardware acceleration.

In recent Scopy versions, a pop-up will appear when this issue is detected, 
the preference will be automatically disabled and Scopy will try to restart.  
For older Scopy versions, please do this manually.

