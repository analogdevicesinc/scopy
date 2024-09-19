.. _debugger:

Debugger
================================================================================

This instrument represents the first version of the debugger tool, also available in the
first version of Scopy. It is composed of 2 parts, the `DEVICE SELECTION` and the
`REGISTER MAP SETTINGS`.

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-tool-v1.png
  :alt: Debugger Tool V1
  :align: center

.. topic:: Device Selection

  This part contains information such as the:

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-v1-device-selection.png
    :alt: Debugger version selection
    :align: center

  #. Selected Device
  #. Selected Channel (or `Global` for device specific attributes)
  #. Selected Channel Attribute (or Device Attribute if the channel selection is set to `Global`)
  #. The value of the selected attribute (this can contain constraints such as a specific list of choices)
  #. The Filename of the selected attribute
  #. Attribute `read` button
  #. Attribute `write` button

.. topic:: Register map settings

  This part contains information such as the:

  .. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/doc_resources/resources/debugger/debugger-v1-register-map-settings.png
    :alt: Debugger version selection
    :align: right
    :width: 700

  #. Source
  #. Display mode
  #. Address
  #. Value
  #. Read Button
  #. Write Button
  #. Description
  #. Default Value
