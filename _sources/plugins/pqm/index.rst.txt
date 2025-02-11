.. _pqm_index:


PQM
---------------------------------------------------------------------

This document describes how Scopy adds support for the PQM board, which 
contains as main components an ADE9430 and a MAX32650 (MCU). The device is 
exposed through an IIO context, from which we receive and send our necessary 
information.

The plugin contains 4 instruments: 

.. toctree::
   :maxdepth: 3

   rms
   harmonics
   waveform
   settings
