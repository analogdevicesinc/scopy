.. _faults:


Faults
============================================================================

Overview
---------------------------------------------------------------------


The Faults Instrument represents the diagnostic operation performed on
the AD-SWIOT1L-SL device, more accurately the AD74413R and MAX14906
chips. The numeric value from each device is extracted from specific
registers and each bit represents a potential error or information about
the chip. For more information on the faults meaning, consider checking
the corresponding datasheets.

Instrument layout
---------------------------------------------------------------------
.. image:: https://github.com/analogdevicesinc/scopy/blob/doc_resources/resources/swiot1l/faults.png?raw=true
   :align: center


1. Hexadecimal read Value

 The last value read from each device, in hexadecimal.

2. Reset Stored
 
 Once a fault value is read twice consecutively, the stored bit is set 
 for that fault (meaning the fault is persistent).
 Pressing the RESET STORED button clears all persistent faults.

3. Clear Selection
 
 Each fault can be selected by clicking it. Scopy will display only the 
 faults that are selected. Pressing the CLEAR SELECTION button clears 
 all selected faults. This can also be achieved by individually 
 deselecting each fault.

4. Run/Single Buttons
 
 The Single button performs 1 single read for both AD74413R and 
 MAX14906 faults.
 The Run button performs a polling operation on both AD74413R and 
 MAX14906 every second.

5. Faults Display
 
 Check the “Faults” dedicated section of this page.

6. Faults Explanations
 
 Each fault is explained in this section. The first 4 faults from AD74413R
 (VI_ERR_A, VI_ERR_B, VI_ERR_C and VI_ERR_D) have special meanings based on the
 configuration of the channel. Only one of them will be shown when a specific
 function is selected and all of them when no specific function is selected.
 The explanations are highlighted for the faults that have the active bit set
 (i.e. were active on the last read).

Faults
---------------------------------------------------------------------

Each fault has 3 sections:
 - **Stored Bit section:**
 
 The stored bit represents whether that specific fault has been 
 active twice consecutively, making it a persistent
 bit. This can be cleared using the RESET STORED button.

 - **Active Bit section:**
 
 The active bit represents the value that was last read from the device.
 
 - **Bit index:** 
 
 The index bit is a notation for identifying faults.
 It does not represent  the fault name. The name can be seen by 
 hovering over the fault or by checking the “Faults Explanation” area 
 corresponding to the index.


Tutorial 
---------------------------------------------------------------------

A tutorial will automatically start first time the tool is open. It can
be restarted from tool Preferences.
