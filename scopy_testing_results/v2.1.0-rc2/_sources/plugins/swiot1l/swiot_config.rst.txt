.. _swiot_config:

Configuration
============================================================================

Overview
--------------------------------------------------------------------

.. image:: https://github.com/analogdevicesinc/scopy/blob/doc_resources/resources/swiot1l/config.png?raw=true
   :align: center

The Config Instrument is only available when the AD-SWIOT1L-SL system is
in configuration mode. This mode allows the user to define, for each of
the 4 physical channels, the device that will control the channel and
the function defined for that device. The channel can also be disabled,
in which case it will not be configured.

 1. Channel index
 2. Channel state (enabled/disabled)
 3. Chip configured on that channel
 4. Function selected (chip specific)
 5. Channel to chip mapping 
   
    The AD-SWIOT1L-SL board image contains a mapping of the AD74413R and 
    MAX14906 chips that highlights the connection between each chip and 
    the corresponding configured channel pin.
 6. Mode switch
    
    After configuring the system pressing the Apply button will switch the 
    SWIOT1L to runtime mode. The device will briefly disconnect while the 
    IIO Context is reconfigured and it will automatically reconnect to 
    the AD-SWIOT1L-SL board.
    
Upon successful connection a different set of instruments will be available 
to operate and control the system.

Tutorial 
--------------------------------------------------------------------------------

A tutorial will automatically start first time the tool is open. It can
be restarted from tool Preferences.
