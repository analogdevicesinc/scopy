.. _swiot1l_index:


AD-SWIOT1L-SL plugin
================================================================================

Overview
---------------------------------------------------------------------

The Scopy `AD-SWIOT1L-SL <https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation-boards-kits/ad-swiot1l-sl.html>`_ plugin is responsible with the operation and
control of the AD-SWIOT1L-SL platform. The Scopy AD-SWIOT1L-SL
plugin, while based on the Scopy application infrastructure, is tailored
to provide all the specific functionalities of this system.

.. note::
   For an overview of the AD-SWIOT1L-SL system, check the complete user guide: `AD-SWIOT1L-SL User <https://wiki.analog.com/resources/eval/user-guides/ad-swiot1l-sl>`_

.. warning::
   For maximum compatibility, please use Scopy v2.0 or newer.
   The AD-SWIOT1L-SL system is not compatible with Scopy v1.4.1 or older.

.. note::
   Check the following `AD-SWIOT1L-SL video series <https://www.analog.com/en/resources/media-center/videos/video-series/getting-up-running-ad-swiot1l-sl-series.html>`_

Software setup
---------------------------------------------------------------------

Download the installer for your Operating System from the Scopy `latest release <https://github.com/analogdevicesinc/scopy/releases/tag/v2.0.0>`_.

Use the :ref:`install guide <user_guide>` for detailed information.


Launching the application
---------------------------------------------------------------------
.. video:: https://github.com/analogdevicesinc/scopy/raw/doc_resources/resources/swiot1l/connect.mov

Scopy is able to control and operate multiple devices. In order to use
it for the AD-SWIOT1L-SL system, the user should click the “Add
Device”(+) button displayed on the front page of the application.

The video attached above demonstrates the process of connecting to the
board. First it will scan the network in order to detect the device. If
the device is connected to the network and able to be scanned, it will
appear in the list as **ip:analog.local**. If the device is not
available in the list, but the IP is known, the user can manually write
the IP in the URI textbox and proceed forward.

.. note::
   The default static IP for all the boards is **192.168.97.40**

.. note::
   Please check the no-OS documentation for details on the `network configuration for AD-SWIOT1L-SL <https://github.com/analogdevicesinc/no-OS/tree/main/projects/swiot1l#network-connection-for-the-swiot1l-default-firmware>`_

Once found and verified Scopy will present a list of compatible
instruments. The list of instruments is explained below, but during this
step the user can enable or disable Scopy functionalities for this
board.

Going forward the homepage of the board is displayed. The homepage
displays some basic information about the board, including its network
IP. It also contains three buttons. One will perform the connection to
the device, the second one performs identification and will blink a led
on the board and the third one will start a quick tutorial for Scopy
first time users. The tutorial is recommended in addition to this user
guide as it will highlight and explain the user interface controls for
the SWIOT1L available instruments.

.. video:: https://github.com/analogdevicesinc/scopy/raw/doc_resources/resources/swiot1l/tutorial-config.mov


Power Supply Options
---------------------------------------------------------------------
.. image:: https://github.com/analogdevicesinc/scopy/blob/doc_resources/resources/swiot1l/ps-warning.png?raw=true
   :align: center

This warning may appear in any of the instruments that interact with
AD-SWIOT1L-SL. The user should check the external power supply switch
position and verify which option is used to power the system.

.. note::
   For more details on the power supply options, check the hardware user guide: `AD-SWIOT1L-SL Hardware User Guide <https://wiki.analog.com/resources/eval/user-guides/ad-swiot1l-sl/hardware>`_


System operation modes
---------------------------------------------------------------------
**Configuration mode:** 
   When the system is powered up it will enter the Configuration
   mode. This mode allows the user to choose device and function
   configuration for each of the 4 physical channels. When entering the
   Configuration mode, everything is reset to default values. Other than
   the initial state after power up, the user can enter this mode by using
   the **Config** button available in all the runtime instruments of Scopy
   (instruments: AD74413R, MAX14906, Faults).

**Runtime mode:**
   In order to enter the Runtime mode, the user needs to configure the required
   channels and use the **Apply** button provided in the Config Instrument.
   In this mode the physical channels are configured and can be used to
   control or acquire data. If any changes are needed, the user can go back
   into Configuration mode by using the **Config** button mentioned above.

More details on switching through the board states are provided on
specific instrument documentation.

.. note::
   Switching between operation modes might take some time depending on the
   network status. When switching from one mode to another the board is
   briefly disconnected from Scopy, that is because the IIO Context that it
   relies on needs to be destroyed and recreated based on the selected
   configuration.

.. toctree::
   :maxdepth: 3

   swiot_config
   ad74413r
   max14906
   faults
   ../registermap/registermap
