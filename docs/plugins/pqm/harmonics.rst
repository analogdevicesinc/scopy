.. _pqm-harmonics:

Harmonics
================================================================================

The main purpose of this instrument is to display and graphically represent the 
voltage and current harmonics or inter harmonics. Each time new data is 
available, the tool updates accordingly.

The instrument contains 3 sections: 

#. Total Harmonic Distortion (THD). 

#. Harmonics table.

#. Harmonics plot.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/harmonics.png
    :align: center

User Guide 
--------------------------------------------------------------------------------

#. From the menu on the left, you can select the desired instrument. To the 
right of the name, there is a button that allows you to start or stop data 
acquisition.
#. Clicking the settings button brings up the settings menu.
#. Settings menu.
#. By pressing the "Single" button, a single batch of data is acquired.
#. By pressing the "Run" button, continuous data acquisition starts.

.. topic:: Settings menu

    .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/harmonicsSettings.png
        :alt: alternate text
        :align: right

    #. Harmonics type: Select the type of harmonics (harmonics or inter 
       harmonics).
    
    #. Active channel: Select active phase to display on the plot. 
       
        * At any given time, only the values of a single phase can be displayed 
          on the plot. 
        
        * The active phase on the plot can also be changed by clicking on the 
          cells in the table. 
        
        * Additionally, if only certain values from a row are selected, only 
          those values will be displayed on the plot.

In the next image, you can see that the plot displays only the harmonic 
values with degree ranging from 3 to 14.
    
    .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/harmonicsRange.png
        :align: center