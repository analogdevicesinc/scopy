.. _logic_analyzer:


Logic Analyzer
================================================================================

**Video guide:**
    
.. video:: https://www.youtube.com/watch?v=zWX7VnKDYq4  
    
|


General 
----------------------------------------------------------------------------

To switch to this instrument click on the Logic Analyzer button from the left 
menu.

This instrument can capture only in single mode by pressing the Single button.

The logic analyzer instrument consists of a menu containing settings for the 
acquisition, a channel enumerator, decoder enumerator, export settings and 
another menu for the current selected channel


.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_1.png
    :align: center

1.General Settings menu

2.Last opened menu

3.Channel Settings menu

4.Cursors menu

5.Trigger menu

6.Decoder Table button

7.Print button

8.Group button

9.Scroll plot handle


Channel & Decoder Manager
----------------------------------------------------------------------------

1. Enable channel: By toggling the checkbox the channel will or will not be 
   visible on the plot

2. Select a trigger configuration for the channel: The dropdown will allow to 
   select a trigger for the channel or none

3. Add a new decoder: Select a decoder from a list. The selected decoder will be 
   visible in the decoder enumerator

4. Remove decoder: Allows removing of a decoder

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_2.png
    :align: center


General settings
----------------------------------------------------------------------------

1. Sample Rate: This spinbox will allow setting the sample rate of the Logic 
   Analyzer

2. Number of Samples: This spinbox will allow setting how many samples the Logic 
   Analyzer will capture

3. Delay: This spinbox will set the time trigger delay

4. OneShot/Stream: select either we wait for the whole set of samples to be 
   captured, or capture sequentially smaller chunks and plot them

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_3.png
    :align: center


Export
----------------------------------------------------------------------------

The Logic Analyzer can export current data in .csv (Comma-separated values) and 
.vcd (Value change dump) format. Using the “Export All” switch you can 
select and export data from all the available channels or you can create a 
custom selection using the dropdown. After deciding which channels should be 
exported, click “Export” and choose a file. The exported .csv files are 
compatible with instruments throughout the application, so you could load the 
file in the Pattern Generator.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_4.png
    :align: center


Channel settings
----------------------------------------------------------------------------

To view the channel selected settings select from the bottom menu the 
“Channel Settings”. In order to select a channel the handle of the channel 
should be “double clicked”. In this menu the name of the channel can be 
changed. The assigned trigger option for it can be modified and we can also 
play with the trace height which is in pixels to better see or fit the signal 
on the plot

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_5.png
    :align: center

Decoder settings
----------------------------------------------------------------------------

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_6.png
    :align: center

After adding a decoder from the general menu we can view it by selecting it on 
the plot and going to the channel settings menuScopy Logic Analyzer 

1. The channels from which the decoder will decode data need to be assigned 
   here. The channels that are required are marked with an asterix(*)

2. On top of a decoder we can stack another one by selecting from a list of 
   compatible decoder

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_7.png
    :align: center

A stacked decoder can be removed by selecting the x button next to the name of 
it. There is no limit to stacking decoders, only that one decoders output is 
the next ones input and some decoders might not send further data.



Grouping Channels and Decoders
----------------------------------------------------------------------------

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_13.png
    :align: center


1. The group button when enabled will allow to select multiple channels from 
   the handles area (the group button when enabled will become a done button that 
   when clicked will create a group with the selected channels)

2. When grouping channels together select all the channels that you want to be 
   part of a new group from the handles area (Selecting a channel is done by 
   double clicking the handle of the channel on the left side of the plot).

3. Channels can be removed from the group by pressing the red “X” button

4. The channels can be moved around in the group (changed order) by using the 
   move icon


Trigger Settings Menu
----------------------------------------------------------------------------

1. Change the mode from auto to normal

2. Select the logic between each channels condition and the external trigger 
   (and / or)

3. Enable the external trigger

4. Select a source for it

5. Select a condition for the external source


.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_8.png
    :align: center


Decoder Table
----------------------------------------------------------------------------

1. Select decoder

2. Select the leading message type for grouping

3. Set group size

4. Offset groups by annotations amount

5. Case insensitive regex search (press Enter to start searching)

6. Filter annotation types

7. Export all visible data in 2 formats:

    7.1. .txt groups data per annotation applying table filter (like 
         pulseview's decoder export)

    7.2. .csv groups data per sample applying table filter and search

8. Group info can be enabled/disabled in preferences


.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_14.png
    :align: center


Use Cases
----------------------------------------------------------------------------

**Prerequisites**
----------------------------------------------------------------------------

    1. Connect ADALM2000 to your computer via USB.
    2. Start Scopy and connect to the device.
    3. From the left menu, choose the Logic Analyzer.

|

**Enable & run multiple channels**
----------------------------------------------------------------------------

    1. Enable 8 channels using the channel manager located in the left side of 
       the plot. To disable a channel use its blue checkbox.
    
    2. To acquire 200 ms of data, change the sample rate to 100 ksps, and the 
       number of samples to capture to 20k samples.
    
    3. Start the acquisition using the “Single” button. After 200ms you 
       should see 8 signals on the plot.

|

**Interaction with Scopy Pattern Generator**
----------------------------------------------------------------------------

    1. From the left menu, choose the Pattern Generator tool.
    
    2. Select channel 0 and 1 and create a group.
    
    3. Select the group and set the pattern to i2c. Set the following 
       parameters:

        3.1. frequency: 5khz

        3.2. data: “abcd”.
    
    4. Start the Pattern Generator using the top right button.
    
    5. In the Logic Analyzer add a i2c decoder
    
    6. Select this decoder and set the SCL channel to 0 and SDA to 1
    
    7. In the general settings set the sample rate to 1Msps and the acquired 
       number of samples to 50k samples

|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_9.png


To zoom in click and select a region of interest. To zoom out right click 
should be used

|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_10.png

|

Now the decoded data should be more visible

|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_11.png

|

To enable the cursors toggle the checkbox in the bottom menu

|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/logicAnalyzer/logic_analyzer_new_12.png

|


1. Move the handles to the desired position

2. Cursor readouts

3. Change the position of the readouts on the plot

4. Change the transparency of the readouts
    
5. Toggle the cursors visible
    
6. Lock the cursors (moving one cursor will move the other one to keep the 
   set distance between them)

