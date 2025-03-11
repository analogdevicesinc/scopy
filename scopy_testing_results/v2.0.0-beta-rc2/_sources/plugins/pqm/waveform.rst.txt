.. _pqm-waveform:

Waveform
================================================================================

The main purpose of this instrument is to display the waveform of the voltage 
and current for each phase. In one second, 10 cycles should be displayed at a 
frequency of 50 Hz, or 12 cycles at a frequency of 60 Hz.

The instrument is divided into two sections: 

#. Voltage plot: Shows voltage waveforms (top).
#. Current plot: Shows current waveforms (bottom).

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/waveform.png
    :align: center

User Guide 
--------------------------------------------------------------------------------

#. From the menu on the left, you can select the desired instrument. To the 
right of the name, there is a button that allows you to start or stop data 
acquisition.
#. Clicking the settings button brings up the settings menu.
#. By pressing the "Single" button, a single batch of data is acquired.
#. By pressing the "Run" button, continuous data acquisition starts.

.. topic:: Settings menu

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/waveformSettings.png
    :alt: waveform settings
    :align: right
  
  #. Timespan: The time interval for displaying data on the plot can be 
     set to a value between 20 ms and 10 seconds.

  #. Triggered by: If we are in trigger mode for displaying data on the 
     plot, we can select which phase will be used as the basis for the display (by 
     default is the phase Ua). This menu option becomes inactive when rolling mode 
     is activated or data acquisition starts.
  
  #. Rolling mode: This switch is used to activate/deactivate rolling 
     mode (by default is off).
