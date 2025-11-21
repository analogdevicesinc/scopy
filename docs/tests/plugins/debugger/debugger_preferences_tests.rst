Debugger Preferences - Test Suite
====================================================================================================

General Description: Debugger preferences are settings that control the behavior of the debugger instrument. You can access the debugger preferences by going to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. The debugger instrument preferences are:

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

.. _pluto-usb:

**Pluto.Usb:**
        - Open the Scopy application
        - Connect the PlutoSDR to the computer via USB
        - Type the URI of the PlutoSDR in the URI field (if you don't know the URI, just type "ip:192.168.2.1")

Test 1: Control debugger version
----------------------------------------------------------------------------------------------------

**UID**: TST.DBG.PREF.CHANGE_VERSION

**RBP:** P1

**Description**: This option allows you to use the new debugger plugin. The new plugin is more stable and has more features than the old plugin. The old plugin is still available for use, but it is recommended to use the new plugin. When this option is checked, the new plugin is used. When it is unchecked, the old plugin is used. After changing this option, a restart of Scopy is required for the change to take effect.

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Click on the connected Pluto device and connect to it. Select the Debugger plugin. By default, the IIO Explorer (Debugger v2) should be displayed.
                - **Expected Result:** The IIO Explorer (Debugger v2) should be displayed. If you notice the navigation bar with devices and attributes on the left side, the IIO Explorer is displayed.
                - **Actual result:**

..
        Actual test result goes here.
..

        2. Go to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. Uncheck the "Use debugger V2 plugin" option.
                - **Expected Result:** A 'Restart' button should appear at the bottom of the preferences page. Click on the 'Restart' button. Scopy should restart.
                - **Actual result:**

..
        Actual test result goes here.
..

        3. Click on the connected Pluto device and connect to it. Select the Debugger plugin.
                - **Expected Result:** The Debugger v1 plugin should be displayed. If you notice 2 sections, 'DEVICE SELECTION' and 'REGISTER MAP SETTINGS', the Debugger v1 plugin is displayed.
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Repeat step 2, but this time check the "Use debugger V2 plugin" option.
                - **Expected Result:** A 'Restart' button should appear at the bottom of the preferences page. Click on the 'Restart' button. Scopy should restart.
                - **Actual result:**

..
        Actual test result goes here.
..

        5. Click on the connected Pluto device and connect to it. Select the Debugger plugin.
                - **Expected Result:** The IIO Explorer (Debugger v2) should be displayed. If you notice the navigation bar with devices and attributes on the left side, the IIO Explorer is displayed.
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


Test 2: Include debug attributes in IIO Explorer
----------------------------------------------------------------------------------------------------

**UID**: TST.DBG.PREF.INCLUDE_DEBUG_ATTRIBUTES

**RBP:** P2

**Description**: This option allows you to include IIO debug attributes in the IIO Explorer. When this option is checked, debug attributes are included in the IIO Explorer. When it is unchecked, debug attributes are not included in the IIO Explorer. After changing this option, a restart of Scopy is required for the change to take effect.

**Test prerequisites:**
        - :ref:`TST.DBG.EXPLR.NAV <tst-dbg-explr-nav>`

**Preconditions**:
        - Use :ref:`pluto-usb <pluto-usb>` setup.
        - OS: ANY

**Steps**:
        1. Click on the connected Pluto device and connect to it. Select the Debugger plugin. By default, the IIO Explorer (Debugger v2) should be displayed.
        2. Go to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. Check the "Include debug attributes in IIO Explorer" option. (if it is checked, leave it as it is)
        3. Go back to the Debugger plugin and notice the navigation bar on the left side. Double click on ad9361-phy and scroll down a bit.
                - **Expected Result:** Among the final device attributes of the ad9361-phy device, you should see the debug attributes. They should start with the 'adi,' prefix.
                - **Actual result:**

..
        Actual test result goes here.
..

        4. Go back to the Preferences page and uncheck the "Include debug attributes in IIO Explorer" option.
        5. Restart Scopy.
        6. Click on the connected Pluto device and connect to it. Select the Debugger plugin.
        7. Look at the navigation bar on the left side. Double click on ad9361-phy and scroll down a bit.
                - **Expected Result:** The debug attributes should not be displayed among the final device attributes of the ad9361-phy device. (i.e., no attributes should start with the 'adi,' prefix)
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

