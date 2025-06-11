FILES:

1. sineGenerator.py - Generates samples for the pqm device channels.  The data is written in "data.bin".

2. pqmDataWriter.py - Generates random values for the attributes of the channels (rms, angle, harmonics and inter_harmonics). 

3. pqm1.xml - Contains the context structure of the pqm device.

Steps:

1. Open a terminal in this folder.
2. Run 'iio-emu generic pqm1.xml iio:device0@data.bin' 
   or 
   Use the Emu tab from Scopy with: 
   	- XML Path: /absolutePathTo/pqm1.xml
   	- RX/TX devices: iio:device0@/absolutePathTo/data.bin
3. Run sineGenerator.py
4. Run pqmDataWriter.py
5. Connect to the device through Scopy.

