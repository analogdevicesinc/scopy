.. _scripting_tool:

Scripting Tool
==============

The Scopy Scripting Tool allows users to automate and extend Scopy’s functionality using JavaScript. Scripts can interact with devices, automate measurements, and control Scopy’s UI and instruments.

Features
--------

- **Script Editor:** Write and edit JavaScript scripts directly in Scopy.
- **Load/Save:** Open and save scripts to files.
- **Run:** Execute scripts (stopping a running script is not supported).
- **Console Output:** View script output and errors in a dedicated console.
- **Command Line Input:** Enter and execute single-line commands interactively.

How Script Execution Works
--------------------------

- Scripts are executed using Qt’s built-in `QJSEngine` (ECMAScript/JavaScript engine). See the official documentation: https://doc.qt.io/qt-5/qjsengine.html
- The scripting tool provides a code editor, console, and run control.
- Scripts can be run from the editor or loaded from file.

Scripting Examples
--------------------------

1. Connect to a device and iterate through its tools

.. code-block:: javascript

    // Find and connect to the first available device
    let devices = scopy.getDevicesName();
    if (devices.length !== 0) {
        let devID = scopy.addDevice(devices[0], "");
        let connected = scopy.connectDevice(devID);
        if (connected) {
            let tools = scopy.getTools();
            for (let i = 0; i < tools.length; i++) {
                // Tool iteration
            }
            scopy.disconnectDevice(devID);
        }
    }

2. Connect to a device, switch to regmap, and read register 0x00

.. code-block:: javascript

    // Find and connect to the first available device, switch to RegMap, and read register 0x00
    let devices = scopy.getDevicesName();
    if (devices.length !== 0) {
        let devID = scopy.addDevice(devices[0], "");
        let connected = scopy.connectDevice(devID);
        if (connected) {
            let switched = scopy.switchTool(devID, "RegMap");
            if (switched) {
                let value = regmap.read(0x00);
                // Register value read
            }
            scopy.disconnectDevice(devID);
        }
    }

3. With a connected device: in the register map, read the first register, change its value to 1, read again, then restore the original value (with msleep)

.. code-block:: javascript

    // Assumes device is already connected and RegMap tool is active
    let originalValue = regmap.read(0x00);

    regmap.write(0x00, 1);
    scopy.msleep(1000);

    let newValue = regmap.read(0x00);

    regmap.write(0x00, originalValue);
    scopy.msleep(1000);

    let restoredValue = regmap.read(0x00);

Command Line Scripting
----------------------

You can also run scripts from the command line:

.. code-block:: bash

    scopy --script myscript.js

Or run a list of scripts:

.. code-block:: bash

    scopy --script-list script1.js script2.js


.. warning::

    Stopping a running script is not supported.
