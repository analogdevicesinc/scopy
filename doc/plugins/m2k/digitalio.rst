.. _digitalio:

Digital IO
================================================================================


    **Video guide:**

.. video:: https://www.youtube.com/watch?v=9cYstnAoUpk  

    
|


General description
--------------------------------------------------------------------------------

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/digitalio/scopy_2018-05-15_16-46-33.png
    :alt: alternate text
    :align: right


1. Digital channel number - the physical pin of the M2K
2. Channel input state - indicates the input read by the M2K
3. Channel direction - indicates whether the channel is set to input/output
4. Channel output state - indicates the output value configured of the M2K


Use Cases
--------------------------------------------------------------------------------

    **IO operation**

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/digitalio/scopy_2018-05-15_16-51-11.png
    :alt: alternate text
    :align: right

1. Run instrument

2. Connect digital channel 0 to channel 7

3. Start digital IO instrument

4. Monitor channel 0 using voltmeter/scope/logic analyzer

5. Set channel 0 as output by clicking the direction button Channel 7 input 
   state is the same as channel 0 output state Voltmeter shows the same state 
   (5V/0V) as channel 0 output state


6. Change channel 0 output state by clicking it (multiple times) Channel 7 
   input state is the same as channel 0 output state Voltmeter shows the same 
   state (5V/0V) as channel 0 output state

--------------------------------------------------------------------------------

  **Grouped operation**

1. Make the following connections

* Channel 0 → Channel 8
* Channel 1 → Channel 9
* Channel 2 → Channel 10
* Channel 3 → Channel 11
* Channel 4 → Channel 12
* Channel 5 → Channel 13
* Channel 6 → Channel 14
* Channel 7 → Channel 15

2. Start digital IO instrument

3. Also monitor these connections with a logic analyzer (use breadboard to 
   split the connection)

4. Set DIO 0-7 to Grouped interface


5. Set all channels DIO8-15 to output

6. Set each DIO8-15 output states at random

DIO 0 – 7 grouped value should be the binary value of channels 8-15. Logic 
analyzer should show the same results

7. Set all channels 8-15 to input

8. Set Group direction to output

9. Set Group value to a random value

Channels 8-15 input state should represent that value in binary

Logic analyzer should show the same results

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/digitalio/scopy_2018-05-15_17-11-24.png
    :align: center

--------------------------------------------------------------------------------

   **Interaction with pattern generator**

1. Start digital IO instrument

2. Generate clock signals on channels 0 and 8

In the digitalIO the group as well as channel 8 should have a red highlight 
indicating that the pattern generator has ownership of the channels.

The user cannot interact with those channels. Logic analyzer shows clock signal 
correctly generated at channels 0 and 8

3. Connect channel 10 to channel 11

4. Set channel 10 as output and channel 11 as input

5. Modify channel 10 output state

Channel 11 input state follows channel 10 output state

Rest of the channels work properly. The user can set direction and output state 
and can read input state

6. While running, set a clock signal on channel 10 in the pattern generator 
   Channel 10 has a red highlight, the user cannot interact with it anymore. 
   Channel 11 input state might flicker indicating that a signal can be read from 
   channel 11.

Logic analyzer shows a new clock signal generated at channel 10 which can also 
be read on channel 11

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/m2k/digitalio/scopy_2018-05-15_17-13-47.png
    :align: center

7. Stop the pattern generator

Red highlight is removed from the digitalIO, the channels now work properly.


