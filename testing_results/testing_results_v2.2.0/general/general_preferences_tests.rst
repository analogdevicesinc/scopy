.. _general_preferences_tests:

General Preferences - Test Suite
==================================

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

Setup environment:
------------------

.. _m2k_emu_setup:

**M2K.emu:**
   - Press the add device button
   - Enable emulator for adalm2000
   - Verify and add device with M2KPlugin and ADCPlugin enabled

.. _pluto_emu_setup:

**Pluto.emu:**
   - Press the add device button
   - Enable emulator for pluto
   - Verify and add device with ADCPlugin enabled

.. _pluto_emu_usb_setup:

**Pluto.usb:**
   - Connect a pluto device via usb to your PC
   - Connect to the device in Scopy with ADCPlugin enabled


Test 1 - Save/Load Scopy session
--------------------------------------------------

.. _TST.PREF.SAVE_LOAD_SESSION:

**UID:** TST.PREF.SAVE_LOAD_SESSION

**RBP:** P0

**Description:** Manage sessions to retain or load overall application
states (only applies to plugins which have this implemented).

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`M2K.emu<m2k_emu_setup>`
   - OS: ALL

**Steps:**
   1. Set preference to enabled
   2. Open Oscilloscope tool
   3. Modify the time base value to 1s
   4. Close Scopy
   5. Open the app using the same setup and check if the time base
       - **Expected result:** time base is 1s
       - **Actual result:**

..
  Actual test result goes here.
..

   6. Redo all the steps with the preference disabled
       - **Expected result:** time base is the default value
       - **Actual result:**

..
  Actual test result goes here.
..

   7. Press the "Settings file location" open button within the preference menu
       - **Expected result:** should open a file browser containing 
         .ini files for every plugins' session saved
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


Test 12 - Auto-connect to previous session
--------------------------------------------------

.. _TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION:

**UID:** TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION

**RBP:** P0

**Description:** Automatically reconnect to the last used devices on startup.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**  
   - :ref:`Pluto.usb<pluto_emu_usb_setup>`
   - OS: ANY  

**Steps:**  
   1. Set preference to enabled.  
   2. Connect to the device and then close the application.  
   3. Open Scopy.  
       - **Expected result:** Scopy should automatically connect 
         to the Pluto device.    
       - **Actual result:**

..
  Actual test result goes here.
..

   4. Redo all the steps with the preference disabled.
       - **Expected result:** Should not connect automatically.
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


Test 21 - Reset settings and plugins to default
--------------------------------------------------

.. _TST.PREF.RESET_SETTINGS_DEFAULT:

**UID:** TST.PREF.RESET_SETTINGS_DEFAULT

**RBP:** P0

**Description:** Resets all settings and plugins to default values.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`M2k.emu<m2k_emu_setup>` 
   - OS: ANY  

**Steps:**  
   1. Change general preferences and M2K preferences.  
   2. Press the "Reset" button and the "Restart" to restart the app.  
       - **Expected result:** All changed preferences should have 
         the default values.    
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


