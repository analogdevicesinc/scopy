.. _dac:

DAC plugin
================================================================================

The DAC plugin is used to interface with IIO DACs that implement the IIO buffer
mechanism or a DDS mechanism. The plugin consists of an instrument with two
data generation modes.

The plugin is compatible with contexts that have at least one IIO device that 
implements an output IIO buffer or at least one DDS (ALTVOLTAGE) output channel.

Both generation modes feature a few common items in their layout.

1. Info button

 The top left side information button will open a pop-up window containing
 two buttons: **Tutorial** and **Documentation**.

 The Documentation button will open this page in a browser and close the 
 pop-up window.

 The Tutorial button will start a tutorial that will guide the user through
 the main features of the plugin.

2. Right side menu

 The right side menu is controlled by device or channel buttons. If only one
 DAC device is detected in the current context, its button will be available
 at the bottom of the instrument. If more DACs are detected, a left side
 device list will appear allowing the user to control which is the active
 and visible one at the moment. All device buttons can control the right 
 side menu and all their IIO attributes are displayed in there.

 All the buffer capable channels of an IIO device are also capable to control
 the right side menu. More on these channel buttons can be found below in 
 the buffer mode section.


Buffer mode
---------------------------------------------------------------------

.. image:: https://github.com/analogdevicesinc/scopy/blob/doc_resources/resources/dac/dac-buffer.png?raw=true
   :align: center

If the IIO device implements a IIO buffer mechanism, this mode will be active 
and visible by default.
Find all the sections of the instrument described below.

1. Dac Buffer Settings

 Browse and choose a .CSV file containing the channel data you want the 
 DAC to output. 
 Use the "Load" button to check and parse the provided file.
 The file is interpreted as one column per channel so make sure to structure
 your file accordingly. The file should not contain any comments or header 
 data as loading will fail.

 If loading is successful the "File Size" will be populated with the parsed 
 file size and a new section will appear below.

 .. note::
    Only CSV (Comma separated values) accepted for now.

2. Channels

 List of detected IIO channels that are buffer capable. These can be 
 enabled or disabled using their left side check.

3. Repeat data

 Choose whether to repeat the data columns loaded from file or not.
 If enabled, the data columns will be repeated on all enabled channels, 
 without requiring the user to duplicate the data sets in the file to
 match the number of enabled channels.

 By default it is set to true.

4. Cyclic

 Choose whether to push a cyclic IIO buffer or stream non-cyclic buffers.
 If disabled, a new section containing "Buffer size" and "Kernel buffers"
 will appear.

 By default it is set to true.

5. File Size

 The size of the loaded file. It is populated once loading is successful
 and can be changed to a lower value, truncating the data loaded from file.

6. Run Button

 Start output generation on the currently enabled channels. If anything 
 is invalid in the setup it will appear in the bottom "Console Log" once 
 the Run Button is pressed. 

 Some scenarios might be:
  - no channels are enabled
  - there are more enabled channels than data available in the file
  - errors appeared while trying to configure the IIO Buffer
  - the combined enabled channels are not compatible (for I/Q channels)

7. Data Configuration

 This section allows the user to scale the data before output.

8. Console Log

 All errors or status messages are displayed here.

9. File Configuration

 This section is visible only when the buffer is non-cyclic. It contains a 
 control for buffer size and one for kernel buffers. Both are automatically
 computed and populated when the file size changes.
 The buffer size can be in range [16, file size] while the kernel buffers
 can be in range [1, 64].


DDS mode
---------------------------------------------------------------------

.. image:: https://github.com/analogdevicesinc/scopy/blob/doc_resources/resources/dac/dac-dds.png?raw=true
   :align: center

If the IIO Context contains an IIO Device with DDS capable channels, this
mode will become available and will provide an automatically scanned structure
for available TX channels with I/Q corresponding channels.

1. TX header section
 
 This section contains the name of the TX. It can be TX1..TXn or it can be just 
 the index. This is determined based on the detected channels, whether 
 I/Q channels or just simple tx channels are found.

 The next area in this section is the DDS MODE selector. The options available are:
  - disabled
  - one tone
  - two tones
  - independent I/Q control

 The last item of this section is the Read button which will refresh the values
 of all attributes below.

2. Tone area

 Based on the selected DDS MODE the layout of this area will change, having 
 visible only the number of tones selected in the previously mentioned
 selector.

 Each tone has three modifiable attributes for "frequency", "phase" and "scale".
 
 .. note::
   For scale, "-Inf dB" is equivalent with scale=0 or a disabled channel.
