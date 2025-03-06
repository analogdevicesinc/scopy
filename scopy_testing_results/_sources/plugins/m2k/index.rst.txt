.. _m2k_index:


ADALM2000
================================================================================

Overview
---------------------------------------------------------------------

The `ADALM2000 
<https://www.analog.com/en/resources/evaluation-hardware-and-software/evaluation
-boards-kits/ADALM2000.html>`_ (M2K) Active Learning Module is an affordable 
USB-powered data acquisition module that takes the capabilities of the 
`ADALM1000 <https://www.analog.com/ADALM1000>`_ (M1K) Active Learning Module to 
the next level. With 12-bit ADCs (at 100MSPS) and DACs (at 150MSPS), the 
ADALM2000 with the Scopy software, brings the power of high performance lab 
equipment to the palm of your hand, enabling electrical engineering students 
and hobbyists to explore signals and systems into the tens of MHz without the 
cost and bulk associated with traditional lab gear. The ADALM2000, when coupled 
with Analog Devices' Scopy™ graphical application software running on a 
computer, provides the user with the following high performance instrumentation:

* Two-channel oscilloscope with differential inputs

* Two-channel arbitrary function generator

* 16-channel digital logic analyzer (3.3V CMOS and 1.8V or 5V tolerant, 100MS/s)

* 16-channel pattern generator (3.3V CMOS, 100MS/s)

* 16-channel virtual digital I/O

* Two input/output digital trigger signals for linking multiple instruments 
  (3.3V CMOS)

* Two-channel voltmeter (AC, DC, ±25V)

* Network analyzer – Bode, Nyquist, Nichols transfer diagrams of a circuit. 
  Range: 1Hz to 10MHz

* Spectrum Analyzer – power spectrum and spectral measurements (noise floor, 
  SFDR, SNR, THD, etc.)

* Digital Bus Analyzers (SPI, I²C, UART, Parallel)

* Two programmable power supplies (0…+5V , 0…-5V)

Introduction
---------------------------------------------------------------------

ADALM2000 is a portable lab. It's more than the combinations of a few parts, 
but to understand the capabilities of the unit, you must understand the 
fundamental operation of each part inside the unit. Depending on who you are, 
and what you want to do, you may have different needs, and different wants. 
It's expected that many people will stop when they solve their immediate 
problem and don't want to go any deeper into the stack. There are 3 main 
categories of users:

   1. `ADALM2000 Users & Students: <https://wiki.analog.com/university/tools/m2k/users>`_

      * ADALM2000 users normally interact with real world analog signals using 
        Scopy or Alice
      
      * Everyone should read this section, as it describes the device, and 
        describes how to get the drivers and host software installed properly.

   2. ADALM2000 Application Developers:

      * Want to write their application interface for the ADALM2000 to run on 
        the PC, but do not need to modify the ADALM2000 firmware or need to know the 
        details on the ADALM2000's inner workings
      
      * Use `scopy's scripting ability <https://wiki.analog.com/university/tools/m2k/scopy/scripting-guide>`_ . 
        Write scripts in javascript and then call scopy via the command line.
      
      * Use `libm2k <https://wiki.analog.com/university/tools/m2k/libm2k/libm2k>`_ 
        to write C++/C#/Python applications that capture and generate data from the host PC.

   1. `ADALM2000 Firmware Developers: <https://wiki.analog.com/university/tools/m2k/developers>`_

      * normally write custom software or HDL (for the FPGA) that run directly 
        on the ADALM2000 device. This may put the ADALM2000 in different modes, and 
        support different external USB devices (including LAN (via USB), or WiFi (vs 
        USB)), extending the capabilities of the device. This includes all the 
        information to compile HDL projects, compile kernels, change to custom USB 
        PID/VID and/or run custom user space applications.

|

It's expected that most people will work their way down through each section, 
reading and skimming most of the content. The content is in a constant state of 
improvement, so if you do have a question, please ask at EngineerZone, or check 
the help and support page.

.. warning::

   All the products described on this page include ESD (electrostatic 
   discharge) sensitive devices. Electrostatic charges as high as 4000V readily 
   accumulate on the human body or test equipment and can discharge without 
   detection.

|

.. warning::
      
   Although the boards feature ESD protection circuitry, permanent damage may 
   occur on devices subjected to high-energy electrostatic discharges. Therefore, 
   proper ESD precautions are recommended to avoid performance degradation or loss 
   of functionality. This includes removing static charge on external equipment, 
   cables, or antennas before connecting to the device.


ADALM2000 plugins: 
---------------------------------------------------------------------

.. toctree::
   :maxdepth: 3

   oscilloscope
   spectrum_analyzer
   network_analyzer
   signal_generator
   logic_analyzer
   pattern_generator
   digitalio
   voltmeter 
   power_supply
