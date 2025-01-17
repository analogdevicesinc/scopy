.. _troubleshooting:

Scopy troubleshooting guide
===========================

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

