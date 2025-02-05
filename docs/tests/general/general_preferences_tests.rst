.. _general_preferences_tests:

General Preferences - Test Suite
==================================

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
   6. Redo all the steps with the preference disabled
       - **Expected result:** time base is the default value
   7. Press the "Settings file location" open button within the preference menu
       - **Expected results:** should open a file browser containing 
         .ini files for every plugins' session saved

**Result**: PASS/FAIL

Test 2 - Double click to attach/detach tool
--------------------------------------------------

.. _TST.PREF.DOUBLECLICK_ATTACH_DETACH:

**UID:** TST.PREF.DOUBLECLICK_ATTACH_DETACH  

**Description:** Tools can be detached from the main window by 
double clicking them in the left menu.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**  
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ALL  

**Steps:**  
   1. Set preference to enabled.  
   2. Double click Oscilloscope tool to detach it.  
       - **Expected result:** Oscilloscope tool should detach from 
         the main window into a new one.  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** Nothing should happen.

**Result**: PASS/FAIL

Test 3 - Save/Load tool attached state
--------------------------------------------------

.. _TST.PREF.SAVE_LOAD_TOOL_STATE:

**UID:** TST.PREF.SAVE_LOAD_TOOL_STATE  

**Description:** Manage sessions to retain tool attached states.  

**Test prerequisites:**
   - Emulator tests
   - Depends on :ref:`Test Double click to attach/detach tool<TST.PREF.DOUBLECLICK_ATTACH_DETACH>`

**Preconditions:**  
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ALL  

**Steps:**  
   1. Set preference to enabled.  
   2. Double click Oscilloscope tool to detach it.  
   3. Close Scopy.  
   4. Open the app using the same setup.  
       - **Expected result:** Oscilloscope is still detached.  
   5. Redo all the steps with the preference disabled.  
       - **Expected result:** Oscilloscope should not be detached.  

**Result**: PASS/FAIL

Test 4 - Double click control buttons to open menu
--------------------------------------------------

.. _TST.PREF.DOUBLECLICK_CONTROL_MENU:

**UID:** TST.PREF.DOUBLECLICK_CONTROL_MENU  

**Description:** Open menu from MenuControlButtons by double clicking them.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ALL  

**Steps:**
   1. Set preference to enabled.  
   2. Open ADC - Time tool.  
   3. Double click Cursors button in the bottom right (
      do not click on the button's icon).  
      
       - **Expected result:** A hovering menu should open above the button.  
   4. Redo all the steps with the preference disabled.  
       - **Expected result:** No menu should open.  

**Result**: PASS/FAIL


Test 5 - Enable OpenGL plotting
--------------------------------------------------

.. _TST.PREF.ENABLE_OPENGL_PLOTTING:

**UID:** TST.PREF.ENABLE_OPENGL_PLOTTING  

**Description:** Use GPU acceleration for rendering plots (OpenGL-based).  

**Test prerequisites:**
   - Emulator tests
   - Depends on Test "Plot FPS"  

**Preconditions:**  
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ALL  

**Steps:**  
   1. Set preference to enabled.  
   2. Set Debug preference "Show plot FPS" to enabled.  
   3. Open Oscilloscope tool and run it.  
       - **Expected result:** The red text inside the plot 
         should contain "OpenGL rendering."  
   4. Redo all the steps with the preference disabled.  
       - **Expected result:** The red text inside the plot
         should contain "Software rendering."  

**Result**: PASS/FAIL

Test 6 - Enable menu animations
--------------------------------------------------

.. _TST.PREF.ENABLE_MENU_ANIMATIONS:

**UID:** TST.PREF.ENABLE_MENU_ANIMATIONS  

**Description:** Toggle visual animations for menu transitions.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**  
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ANY  

**Steps:**  
   1. Set preference to enabled.  
   2. Open Oscilloscope tool and toggle between trigger and channel menus.  
       - **Expected result:** Opening menus should be animated.  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** Opening menus shouldn't have any animations.  

**Result**: PASS/FAIL


Test 7 - Enable the status bar for displaying important messages
-----------------------------------------------------------------

.. _TST.PREF.ENABLE_STATUS_BAR:

**UID:** TST.PREF.ENABLE_STATUS_BAR  

**Description:** Show status messages on the bottom of the app window.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**  
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ANY  

Steps  
   1. Set preference to enabled.  
   2. Do the setup again and look for a message bar when pressing "connect."  
       - **Expected result:** A message about connecting should appear at the bottom of the window.  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** No message should pop up.  

**Result**: PASS/FAIL


Test 8 - Show Grid
--------------------------------------------------

.. _TST.PREF.SHOW_GRID:

**UID:** TST.PREF.SHOW_GRID

**Description:** Display grid lines for better visual referencing on plots.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ANY  

**Steps:**
   1. Set preference to enabled.  
   2. Open ADC - Time tool.  
       - **Expected result:** A grid should be visible on the plot.  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** No grid on the plot.  

**Result**: PASS/FAIL


Test 9 - Show Graticule
--------------------------------------------------

.. _TST.PREF.SHOW_GRATICULE:

**UID:** TST.PREF.SHOW_GRATICULE

**Description:** Display lines for axes centered on 0 points on plots.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ANY  

**Steps:**  
   1. Set preference to enabled.  
   2. Open ADC - Time tool.  
       - **Expected result:** A horizontal and a vertical 
         measurement line should be centered on the 0-axis 
         points (both may not be visible at the same time).  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** The lines should border the entire 
         plot instead of centering on 0 points.  

**Result**: PASS/FAIL

Test 10 - Use Lazy Loading
--------------------------------------------------

.. _TST.PREF.USE_LAZY_LOADING:

**UID:** TST.PREF.USE_LAZY_LOADING

**Description:** Load IIO resources as needed to improve initial load performance.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_usb_setup>`
   - OS: Windows  

**Steps:**  
   1. Set preference to enabled.  
   2. Open Scopy console executable.  
   3. After connecting to Pluto as described in the setup, look 
      in the console for "device connection took:" times.  
   4. Redo all the steps with the preference disabled and 
      compare the elapsed times.  

       - **Expected result:** Elapsed times connecting to some 
         plugins should be lower when lazy loading is enabled.  

**Result**: PASS/FAIL

Test 11 - Use native dialogs
-------------------------------

.. _TST.PREF.USE_NATIVE_DIALOGS:

**UID:** TST.PREF.USE_NATIVE_DIALOGS

**Description:** Use system-native dialog windows.

**Preconditions:**
   - OS: ALL  

**Steps:**  
   1. Set preference to enabled.  
   2. Press the Load button on the left menu.  
       - **Expected result:** A native file browser window should open.  
   3. Redo all the steps with the preference disabled.  
       - **Expected result:** A non-native, Scopy-specific, 
         file browser window should open.  

**Result:** PASS/FAIL

Test 12 - Auto-connect to previous session
--------------------------------------------------

.. _TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION:

**UID:** TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION

**Description:** Automatically reconnect to the last used devices on startup.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**  
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ANY  

**Steps:**  
   1. Set preference to enabled.  
   2. Connect to the device and then close the application.  
   3. Open Scopy.  
       - **Expected result:** Scopy should automatically connect 
         to the Pluto device.  
   4. Redo all the steps with the preference disabled.  
       - **Expected result:** Should not connect automatically.  

**Result:** PASS/FAIL

Test 13 - Font scale
--------------------------------------------------

.. _TST.PREF.FONT_SCALE:

**UID:** TST.PREF.FONT_SCALE

**Description:** Adjust font size within the app (this is experimental, 
so some layout artifacts may appear).  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ANY

**Steps:**
   1. Set preference to 1.45, restart Scopy, and inspect some tools.  
       - **Expected result:** All app text and some buttons should be larger.  
   2. Repeat the steps for font size 1.  
       - **Expected result:** Text should return to default size.  

**Result:** PASS/FAIL


Test 14 - Theme
--------------------------------------------------

.. _TST.PREF.THEME:

**UID:** TST.PREF.THEME

**Description:** Change application theme.  

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>`
   - OS: ANY

**Steps:**
   1. Set preference to a different theme and restart the app.
       - **Expected result:** Themes should change while retaining 
         the same UX and usability in all tools.

**Result:** PASS/FAIL


Test 15 - Language
--------------------------------------------------

.. _TST.PREF.LANGUAGE:

**UID:** TST.PREF.LANGUAGE

**Description:** Change the language of the application interface.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ANY  

Steps
   1. Set preference to a different language and restart the app.  
       - **Expected result:** Some text language should change in the M2K tools.  

**Result:** PASS/FAIL

Test 16 - Connect to multiple devices
--------------------------------------------------

.. _TST.PREF.CONNECT_MULTIPLE_DEVICES:

**UID:** TST.PREF.CONNECT_MULTIPLE_DEVICES

**Description:** Enable connections to multiple hardware and emulator 

**Test prerequisites:**
   - Emulator tests
devices at once.

**Preconditions:**
   - :ref:`Pluto.emu<pluto_emu_setup>` and :ref:`M2k.emu<m2k_emu_setup>`
   - OS: ANY

**Steps:**
   1. Set preference to enabled.  
   2. Connect to 2 devices following the setup.
       - **Expected result:** Two devices should appear in the left 
         menu and be fully functional.  
   3. Repeat the steps with the preference disabled.  
       - **Expected result:** When connecting to the second device, 
         the first one should automatically disconnect.  

**Result:** PASS/FAIL

Test 17 - Regularly scan for new devices
--------------------------------------------------

.. _TST.PREF.REGULARLY_SCAN_NEW_DEVICES:

**UID:** TST.PREF.REGULARLY_SCAN_NEW_DEVICES  

**Description:** Continuously scan and recognize newly connected devices.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.usb<pluto_emu_usb_setup>`
   - OS: ANY

**Steps:**
   1. Connect the Pluto to the machine via USB.  
       - **Expected result:** Pluto should automatically appear in 
         the device list, and a toggle switch for scanning should 
         appear in the top right.  
   2. Repeat the steps with the preference disabled.  
       - **Expected result:** Pluto should not appear automatically, 
         only when pressing the on-demand Scan button.  

**Result:** PASS/FAIL

Test 18 - Session devices
--------------------------------------------------

.. _TST.PREF.SESSION_DEVICES:

**UID:** TST.PREF.SESSION_DEVICES

**Description:** Automatically connect to saved devices.

**Preconditions:**
   - :ref:`Pluto.usb<pluto_emu_usb_setup>`
   - :ref:`"Auto-connect to previous session"<TST.PREF.AUTO_CONNECT_PREVIOUS_SESSION>` preference needs to be enabled
   - OS: ANY  

**Steps:**
   1. After connecting to M2K, go to the preferences and press "Refresh."  
   2. Check the current device's URI.  
   3. Go back to the homepage and forget the device by pressing the red 
      "X" button on the device.  
   4. Close and reopen Scopy.  
       - **Expected result:** Should automatically connect to the URI you 
         checked.  

**Result:** PASS/FAIL

Test 19 - Show plot FPS
--------------------------------------------------

.. _TST.PREF.SHOW_PLOT_FPS:

**UID:** TST.PREF.SHOW_PLOT_FPS

**Description:** Show FPS on plots.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`M2k.emu<m2k_emu_setup>` and :ref:`Pluto.usb<pluto_emu_usb_setup>`
   - OS: ANY  

**Steps:**
   1. Open Oscilloscope and run it.  
   2. Open ADC - Time and run it.  
       - **Expected result:** Should show the FPS on the plot 
         (M2K may look different from other plugins).  
   3. Repeat the steps with the preference disabled.  
       - **Expected result:** No FPS should be visible.  

**Result:** PASS/FAIL

Test 20 - Plot target FPS
--------------------------------------------------

.. _TST.PREF.PLOT_TARGET_FPS:

**UID:** TST.PREF.PLOT_TARGET_FPS

**Description:** Manage the maximum FPS plots should run at 
(this may also affect the IIO acquisition).

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`Pluto.usb<pluto_emu_usb_setup>`
   - "Show plot FPS" needs to be enabled.  
   - OS: ANY  

**Steps:**  
   1. Set the preference to each value and run ADC - Time to check FPS.  
       - **Expected result:** Plot FPS should be capped at the selected value.  

**Result:** PASS/FAIL

Test 21 - Reset settings and plugins to default
--------------------------------------------------

.. _TST.PREF.RESET_SETTINGS_DEFAULT:

**UID:** TST.PREF.RESET_SETTINGS_DEFAULT

**Description:** Resets all settings and plugins to default values.

**Test prerequisites:**
   - Emulator tests

**Preconditions:**
   - :ref:`M2k.emu<m2k_emu_setup>` 
   - OS: ANY  

**Steps:**  
   1. Change general preferences and M2K preferences.  
   2. Press the "Restart" button and restart the app.  
       - **Expected result:** All changed preferences should have 
         the default values.  

**Result:** PASS/FAIL
