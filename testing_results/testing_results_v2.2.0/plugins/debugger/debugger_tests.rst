Debugger - Test Suite
============================================================================================================================

.. note::

    User Guide: https://analogdevicesinc.github.io/scopy/plugins/debugger/index.html

.. note::
    .. list-table:: 
       :widths: 50 30 30 50 50
       :header-rows: 1

       * - Tester
         - Test Date
         - Scopy version
         - Plugin version (N/A if not applicable)
         - Comments
       * - 
         - 
         - 
         - 
         - 

Setup enviroment:
----------------------------------------------------------------------------------------------------------------------------

.. _pluto-usb-debugger:

**Pluto.Usb:**
        - Open the Scopy application
        - Connect the PlutoSDR to the computer via USB
        - Type the URI of the PlutoSDR in the URI field (if you don't know the URI, just type "ip:192.168.2.1")

.. _tst-dbg-explr-load:

Test 1: Loading the debugger
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.DBG.EXPLR.LOAD

**RBP:** P0

**Description**: This test checks if the debugger plugin is loaded when any device is connected to Scopy.

**Preconditions**:
        - Disable the Scopy scan feature
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. After adding the URI, click the "Verify" button
        2. Select the debugger plugin from the list of compatible plugins (the debugger works with any IIO compatible device)
                - **Expected Result:** The plugin list should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Connect to the device and see that the Debugger plugin appears on the left side of the screen (the tool menu)
                - **Expected Result:** The tool menu panel (left side of the screen) should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Disconnect the device and see that the Debugger plugin disappears from the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should not contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Connect the device again and see that the Debugger plugin reappears in the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

        6. Disconnect the device and see that the Debugger plugin disappears from the tool menu
                - **Expected Result:** The tool menu panel (left side of the screen) should not contain the Debugger plugin
                - **Actual result:**

..
        Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


.. _tst-dbg-explr-nav:

