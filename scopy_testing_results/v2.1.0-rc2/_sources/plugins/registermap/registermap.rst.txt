.. _registermap:

Register Map
================================================================================


The Register Map allow access to reading and writing registers for devices 
connected to Scopy.


.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/registermap/registermap4.PNG
    :align: center

|


If the user has an .xml file of the register map that follows the format 
exmplained :ref:`here <xml_file_format>`  the plugin will generate an table 
view matching the .xml file contents 



User Guide 
--------------------------------------------------------------------------------

   1. User can use the address picker to select the address of the register 
      they want to read/write 

   2. Read button will read selected register

   3. The value of a register is shown here once read 

   4. To write a value to the register user can ether 

      * change the value from 3. then press Write button
      * change the value of the bitfields from 7. then press Write button 

   5. Toggle the detailed bitfields (7.) visible

   6. Register map table created based on .xml file. Each row represetns a 
      register the first colum is de register information and the following columns 
      represent the bitfields of the register

   7. For the selected register we display detailed informations about the 
      bitfields and allow user to change values at bitfield value 

   8. Use this menu to select the device you want to use

   9. Search bar can be used to filter the registers. The function will search 
      based on 

      * name or description of register
      * name or description of bitfield

|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/registermap/registermap1.PNG
    :align: center

|


Settings 
--------------------------------------------------------------------------------
|

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/registermap/regmap2.PNG
    :align: right

|

   1. If Autoread is checked when a register is selected a read is 
      automatically done 

   2. User has option to read an interval of registers

   3. If user has a set of values previously read and wants to load them he 
      needs to select the file and press "Write values"

   4. Pressing "Register dump" will save all currently read registers to the 
      selected file


|

Tutorial 
--------------------------------------------------------------------------------

A tutorial will automatically start first time the tool is open. It can
be restarted from tool Preferences.

.. _xml_file_format:


XML file format
--------------------------------------------------------------------------------

The xml file starts and ends with the device name and contains registers. 

The structure of a register is: 

   * Name: the name of the register; 
   * Address: the address of the register; 
   * Description: the description of the register; 
   * Exists: a true or false value; 
   * Width: the size of the register in bits; 
   * Notes: other observations; 
   * Bitfields: a list of Bitfield type elements; 

The structure of a bitfield is: 

   * Name: the name of the bitfield; 
   * Access: the type of access (read and/or write); 
   * DefaultValue: the default value of the bitfield; 
   * Width: the size of the bitfield in bits; 
   * RegOffset: the relative position of the bitfield to the register; 
   * BitOffset; 
   * SliceWidth; 
   * Visibility; 
   * Notes; 
   * Options: the list of possible values for the bitfield 

The structure of an Option type object: 
   * Desctiption: description for the option (will be used in the application) 
   * Value: the value in hexadecimal

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/registermap/regmap5.PNG
    :align: center


.. tip::  
   User can create it's own .xml file for any device and add it to where scopy 
   is build in "/plugins/regmap/xmls/" to be able to see the register map table 


Preferences
--------------------------------------------------------------------------------

This plugin provides functionality to visualize register and bitfield values with color coding. 
Users have the option to choose from the following visualization methods:

1. Change the background color of the register to reflect its value.
2. Change the text color of the register value to reflect its value.
3. Change the background color of the bitfield to reflect its value.
4. Change the text color of the bitfield value to reflect its value.

Users can select any combination of the above options, but only one method can be applied to the register and one method to the bitfield at a time.


