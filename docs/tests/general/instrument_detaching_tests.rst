Instrument Detaching - Test Suite
====================================================================================================

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

General Description:
        - The instrument detaching is the process of removing the instrument from the main window. This is usually done by double clicking the tool name from the tool menu (left side of scopy). The instrument detaching is useful when the user has multiple screens and wants to observe multiple instruments at the same time. The user can detach the instrument from the main window and move it to another screen or another part of the same screen. This feature is not available on Android. Any detached windows will be closed when the main window is closed.
        - When detaching a window, the title bar will include the name of the detached instrument. The user can reattach the instrument either by clicking the X button from the detached window title bar or by double clicking the tool name from the tool menu. The minimize and maximize buttons are also available on the detached window title bar and should work as expected. When reattaching the instrument, the reattached instrument will be the one currently selected. The user can detach multiple instruments at the same time. The detached instruments will be displayed in the order they were detached. Every detached instrument will work exactly as if they were still attached to the main window. The user can detach any instrument that is not the last instrument in the main window. When restarting Scopy, all detached instruments will be detached, just like in the previous session.

Setup enviroment:
----------------------------------------------------------------------------------------------------------------------------

.. _pluto-usb-instrument-detaching:

**Pluto.Usb:**
        - Open the Scopy application
        - Connect the PlutoSDR to the computer via USB

Test 1: Detach and reattach an instrument
----------------------------------------------------------------------------------------------------

**UID:** TST.GEN.INST.DETACH_REATTACH

**RBP:** P3

**Description:** This test checks if the user can detach and reattach an instrument from the main window. The user can detach the instrument by double clicking the tool name from the tool menu (left side of scopy). The user can reattach the instrument either by clicking the X button from the detached window title bar or by double clicking the tool name from the tool menu. The minimize and maximize buttons are also available on the detached window title bar and should work as expected. When reattaching the instrument, the reattached instrument will be the one currently selected.

**Preconditions:**
        - Scopy is installed on the system.
        - Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.
        - Use :ref:`pluto-usb-instrument-detaching <pluto-usb-instrument-detaching>` setup.

**Steps:**
        1. Click on the connected Pluto device and connect to it. Select the 'ADC - Time' plugin.
        2. Double click the 'ADC - Time' tool name from the tool menu.
                - **Expected Result:** The 'ADC - Time' instrument should be detached from the main window. The title bar of the detached window should include the name of the detached instrument.
                - **Actual result:**

..
  Actual test result goes here.
..

        3. Click the square button from the tool menu to start the ADC data aquisition.
                - **Expected Result:** The ADC should start acquiring data. The data should be displayed in the detached window.
                - **Actual result:**

..
  Actual test result goes here.
..

        4. Click the X button from the detached window title bar.
                - **Expected Result:** The 'ADC - Time' instrument should be reattached to the main window. The 'ADC - Time' instrument should be the one currently selected.
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

