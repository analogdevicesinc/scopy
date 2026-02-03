.. _pqm-harmonics:

Harmonics
================================================================================

The main purpose of this instrument is to display and graphically represent the 
voltage and current harmonics or inter harmonics. Each time new data is 
available, the tool updates accordingly.

All widgets in the Harmonics instrument are dockable, allowing users to
customize the workspace by arranging and resizing components.

The instrument contains 4 sections:

#. Total Harmonic Distortion (THD).

#. Harmonics table.

#. Individual current channel plots (Ia, Ib, Ic) - shown by default.

#. Harmonics overview plot - available through a toggle in the settings menu.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/harmonics.png
    :align: center

User Guide 
--------------------------------------------------------------------------------

#. From the menu on the left, you can select the desired instrument. To the right of the name, there is a button that allows you to start or stop data acquisition.

#. Clicking the settings button brings up the settings menu.

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

    #. Log: Enable or disable logging for the Harmonics instrument. When
       enabled, all acquired data will be saved to a log file for later
       analysis.

Overview Plot
--------------------------------------------------------------------------------

The overview plot can be enabled through the settings menu. The user can select
which channel is displayed on the plot, with only one channel visible at a
time. The harmonic degrees to be displayed can be selected by choosing one or
more cells from the harmonics table.

In the image below, only harmonics with degrees ranging from 3 to 14 are
displayed.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/harmonicsRange.png
    :align: center

PQ Events
--------------------------------------------------------------------------------

The PQEvents button is used to notify the user when a specific power quality
event has occurred. When the user clicks on the PQEvents button, it resets and
waits for the next event to occur. All detected events can be viewed in the log
if logging is enabled for the instrument.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/pqm/pqevents.png
    :align: left