.. _user_guide:

Scopy Overview
================================================================================

About 
---------------------------------------------------------------------
Scopy is a multi-functional software toolset with strong capabilities for 
signal analysis.

Download
---------------------------------------------------------------------

.. tip::  

      **Scopy for Windows**

        * Download: `Installer for latest release (Windows 64-bit) <https://github.com/analogdevicesinc/scopy/releases/latest>`_

      **Scopy for Linux**

        * Download: `Scopy Flatpak installer <https://github.com/analogdevicesinc/scopy/releases/latest>`_  
        * Download: `Scopy AppImage executable <https://github.com/analogdevicesinc/scopy/releases/latest>`_

      **Scopy for OSX**

        * Download:`OSX installer <https://github.com/analogdevicesinc/scopy/releases/latest>`_ 

      **Scopy for Android**

        * APK soon available

      **ARM64**

        * Download: `Scopy AppImage executable for ARM64 <https://github.com/analogdevicesinc/scopy/releases/latest>`_

      **ARM32**

        * Download: `Scopy AppImage executable for ARM32 <https://github.com/analogdevicesinc/scopy/releases/latest>`_

.. warning::

  In order to have Scopy working with `ADALM2000 <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/ADALM2000.html>`__
  or `ADALM-Pluto <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/adalm-pluto.html>`__
  please make sure that the Device Drivers are  installed. A guide on this topic can be found in the section:

    * `ADALM2000 and ADALM-Pluto drivers <https://wiki.analog.com/university/tools/pluto/drivers/windows?s[]=adalm&s[]=pluto#windows_drivers>`__


What's new
---------------------------------------------------------------------
Scopy 2.0 is a complete overhaul from the previous version of Scopy introducing a completely new infrastructure and interaction mechanism for any IIO device through generic instruments. While retaining the core functionality of Scopy v1 - support for ADALM2000 devices - this update significantly expands its capabilities. 

Features added in Scopy 2.0
- Multiple device support - control multiple devices from the same Scopy instance
- IIO device support - support for IIO devices through IIO instruments - ADC/DAC/Debugger/Register Map
- ADI Harmonic design style - Light and Dark themes as well as a styling engine
- Plugin system - support for devices other than the ADALM2000

.. warning::
  Preferences and saved Scopy configurations from version 1.x are no longer compatible with Scopy v2.0. For more information click :ref:`here <inifiles>`

Installation
---------------------------------------------------------------------

  **Scopy for Windows**

Once you downloaded the installer, run it and follow all the required steps. 
After completion, system reboot is required.

Video installation guide:

.. video:: https://www.youtube.com/watch?v=894HkVXf7-U


  **Scopy for Linux**

**AppImage**: Make the application executable by running **chmod +x ./Scopy.AppImage** in a command line,
then run Scopy as any other Linux executable. These steps are applicable for ARM64 and ARM32 also.

**Flatpak**: Before downloading and extracting the scopy-Linux-flatpak.zip archive,
you need to setup Flatpak `using this setup guide <https://flatpak.org/setup/>`_ 
for your Linux distribution.

For Ubuntu, you can use the following steps:

.. code-block::

    sudo add-apt-repository ppa:alexlarsson/flatpak

    sudo apt update

    sudo apt install flatpak

    flatpak remote-add --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo


After this, get the Scopy.flatpak file from the downloaded archive and run:

.. code-block::

    flatpak install scopy-Linux-flatpak.flatpak

|

Video installation guide:

.. video:: https://www.youtube.com/watch?v=9qgxmmTrcSE

|


  **Scopy for OSX**

Double click the downloaded .dmg to make its content available. “Scopy” 
will show up in the Finder sidebar and a window showing the content should open 
up.
Drag the application from the .dmg window into Applications to install and wait 
for the process to finish.

Video installation guide:

.. video:: https://www.youtube.com/watch?v=To0ACQ77tkg



  **Scopy for Android** - APK coming soon

Tap on the downloaded .apk file to start the installation process and proceed 
by hitting the “Install” button. Optionally, you may be warned that this 
application comes from unknown sources, hit the “Install anyway” button. 
Now, you can find “Scopy” together with your other apps.

In order to connect `ADALM2000 <https://www.analog.com/ADALM2000>`__  to your 
device, you will need an OTG adapter that will mediate the connection between 
your device and the USB cable, as shown in the image below.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-android-setup-example.png
    :align: center
..

Launch
---------------------------------------------------------------------

Run Scopy from the Desktop Shortcut / Start Menu / Installation Folder.

On Linux, you can also run it using:

.. code-block::

    chmod +x Scopy.AppImage
    ./Scopy.AppImage

OR

.. code-block::

    flatpak run org.adi.Scopy


.. tip::

    If the latter raises issues related to platform display plugins use the following modified command
    to solve it.

    flatpak run --env=QT_QPA_PLATFORM=xcb org.adi.Scopy

    QT_QPA_PLATFORM=xcb ./Scopy.AppImage


Application Overview
---------------------------------------------------------------------

Homepage layout
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-start-page.png
    :align: center
..

The Home view is divided into multiple sections:

Device browser
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 Contains a list of devices (USB/serial/network/emulated) that Scopy can connect to.

 USB devices are scanned automatically at startup when the top right "Scan"
 switch is ON.

Add(+) device page
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 Can be used to connect to devices that are not automatically detected in
 the above mentioned list.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-add-page-iio.png
    :align: center
..

 The interface has two tabs for device configuration:

  - **IIO**: adding and configuring IIO devices.
  - **EMU**: adding and configuring virtually emulated devices using
    the iio-emulator.

 The **IIO** tab includes settings for scanning, connecting, and configuring
 various connected devices.

 **SCAN Section**

 - **Filter**: Allows the user to choose which connection types to scan for.

   If none is selected, the application will scan for all types.

   A new scan is triggered every time one of the following
   options is selected.

    - **Local**: Scans local devices, usually on target.
    - **IP**: Scans network devices.
    - **USB**: Scans USB devices.
 - **Context**: List of currently scanned contexts.
 - **Refresh**: Button to re-scan and detect available devices.

 **SERIAL Section**

 - **Port Name**: Drop-down to select available serial devices.
 - **Baud Rate**: Allows setting the communication speed.
 - **Port Config**: Specifies the data format and parity.
 - **Refresh**: Refreshes the list of available serial devices.

 **URI**

 - **URI Input**: This is where the user specifies the connection URI.
   This field is automatically populated based on the selection from one
   of the previous sections but the user can manually input the URI of
   the required device.
 - **Verify**: Button to verify the connection based on the provided URI.
   This will display an error if no such context is available. On success
   a different page will be displayed allowing the user to configure
   compatible plugins.


 The **EMU** tab includes settings for selecting the emulation mode,
 XML configuration files, RX/TX devices, and URI for the emulated device.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-add-page-emu.png
    :align: center
..

 **DEMO OPTION**:
   - Drop-down menu to select one of the predefined emulated devices.
   - Options are: adalm2000, pluto, swiot, pqm and generic.
     When using the **generic** option, the user must provide a path
     of the XML for the emulated device. If an existing option is
     selected, the XML path will be automatically completed in the
     field.

 **XML PATH**:
   - Specifies the path to the XML configuration file required for the emulation.

 **RX/TX DEVICES**:
   - Specifies the RX (receive) and TX (transmit) device files for the emulation mode.
     These are .bin files containing samples to be used on RX/TX devices.
   - For detailed examples check the `official iio-emu Github docs
     <https://github.com/analogdevicesinc/iio-emu/blob/main/GENERIC_EMULATOR.md>`_ .


 **URI**:
   - The URI is automatically set to **ip:127.0.0.1** or **ip:localhost**.

 **Enable Demo Button**:
   - Validates and activates the demo mode based on the selected configuration.
   - Automatically displays the IIO tab and populates the URI field
     if the EMU validation is successful.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-add-page-plugin-config.png
    :align: center
..

 The add device plugin configuration page will allow the user to
 select compatible plugins for the newly added device.

 By default, the plugins with the highest priority will be selected.
 Clicking the **ADD DEVICE** button will finalize the process and
 create a new device entry in the device browser.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-add-page-done.png
    :align: center
..

.. tip::

  Scopy can be connected to multiple IIO devices simultaneously starting
  with v2.0. All the connected or available IIO devices will be available
  in the device browser.

Instruments Menu
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 This left side area can contain multiple sections of instrument lists,
 one for each connected or selected device. It can also be empty if
 no device is connected or selected. If the device is selected but not
 connected, the instruments will be available only as previews, without
 responding to any interaction.

 A device section has a collapsible section displaying the name and URI of
 the device and will contain a list of available instruments for the selected
 device. The instruments for an IIO device are provided by one or multiple
 plugins based on compatibility. (Example: An IIO device containing only
 ADCs will not have the DAC plugin as an available instrument).
 For more details on the Scopy plugin system and existing plugins please
 check below the "Plugins" documentation section.

Information Window
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

 This area contains the welcome page, add device page and for each device
 a device page.

 The device page will contain context attributes extracted from the
 IIO context, as well as specific buttons or controls that can be provided
 by any compatible plugin (Example: The ADALM2000 has a "Calibrate" button
 which is only specific to this type of device).

Save & Load session
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  Scopy can save or load different configurations in an .ini file format.

Preferences
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Scopy allows multiple application specific user preference configurations
described :ref:`in the Scopy general preferences page <preferences>`

About page
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-about-page.png
    :align: center
..

  Scopy About page can be accessed using the bottom left side **about
  button**. On the right side of the page, about pages specific for each
  plugin are available if necessary. More details can be found in their
  specific plugin documentation section.

  The General About page contains different useful links:

  **Build info** will display the application version, Git commit,
  build date and whether it was created locally or in a continuous
  integration environment, a list of dependencies and their specific
  versions or Git commit hashes, a list of the last 100 commits and
  the Scopy branch.

.. warning::
  These are all important in the debugging process and most of this
  information should be specified when reporting an issue.
..

  **License** link to the open-source LICENSE file in the Github
  repository.

  **Open-source dependencies list** of Scopy includes a number
  of open source libraries, released under their own licenses.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/scopy-deps-licenses.png
    :align: center
..

  **Latest release** for quick access to the latest Github release page.

  **Documentation** for quick access to this documentation.

  **Support forum** for quick access to Engineer Zone.


Connecting to a device
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To connect to any device on the Device Browser click on the device and
then click the **Connect** button in the **Information Window.**

If the connection was established, the device will have a green status line
under it and you will be able to disconnect from it in the same **Information
Window.**

.. |orange_x| raw:: html

  <img src="https://raw.githubusercontent.com/analogdevicesinc/scopy/refs/heads/dev/gui/res/icons/orange_close.svg" alt="Inline image" style="display:inline; vertical-align:middle;">

.. |edit_pen| raw:: html

  <img src="https://raw.githubusercontent.com/analogdevicesinc/scopy/refs/heads/dev/gui/res/icons/edit_pen.svg" alt="Inline image" style="display:inline; scale:80%; vertical-align:middle;">

.. |warning_icon| raw:: html

  <img src="https://raw.githubusercontent.com/analogdevicesinc/scopy/refs/heads/dev/gui/res/icons/warning.svg" alt="Inline image" style="display:inline; width:30px; vertical-align:middle;">

- Clicking the |orange_x| button will remove the device from the list.
- The device title can be modified by hovering over the top right
  corner of the icon and clicking the |edit_pen| pen icon.
- If errors are encountered during the connection phase
  a warning sign |warning_icon| will appear on the top right side of the
  device icon.


User Guides
---------------------------------------------------------------------

Scopy can now interact with multiple hardware devices at a time. Once a device is
selected to be used with the application, a list of instruments that are
available for that device will be visible as a preview. Once connected to a device
the instruments will be enabled and available.

Each instrument can be opened from the left menu. The icon on the right of
the instrument name specifies that the instrument is enabled and provides
a shortcut that allows an instrument to be turned on or off.

For each connected device, there is an entry in the left side menu, above
all the instruments, displaying the **Name** and **URI** of the
connected device.

The instruments menu can be minimized by clicking on the **Scopy** button near
the top-left window.


Detaching Instruments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Scopy provides the detaching into multiple windows feature for each instrument
available, providing a better view/manipulation.

This can be done by double clicking the instrument to detach it.

Please make sure that the **Double click to detach** a tool
option is enabled in the **Preferences** menu.


Instruments Overview
---------------------------------------------------------------------

Scopy provides a list of plugins and instruments
described :ref:`in the Plugins page <plugins>`


Scripting
---------------------------------------------------------------------

User guide on how to use scripts with Scopy:

* `Scopy Scripting Guide <https://wiki.analog.com/university/tools/m2k/scopy/scripting-guide>`_  


Building from source
---------------------------------------------------------------------

.. toctree::
    :titlesonly:

    build_instructions/windowsBuild
    build_instructions/linuxBuild
    build_instructions/macosBuild

Source code
---------------------------------------------------------------------

The source code for the entire application can be found on `github 
<https://github.com/analogdevicesinc/scopy>`_ 


Old documentation
---------------------------------------------------------------------

`Our wiki <https://wiki.analog.com/university/tools/m2k/scopy>`_


