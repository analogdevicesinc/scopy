Debugger Preferences - Test Suite
====================================================================================================

General Description: Debugger preferences are settings that control the behavior of the debugger instrument. You can access the debugger preferences by going to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. The debugger instrument preferences are:

Setup:
        - Pluto.*

Test 1: Control debugger version
----------------------------------------------------------------------------------------------------

UID:
        - TST.DBG.PREF.CHANGE_VERSION

Description:
        - This option allows you to use the new debugger plugin. The new plugin is more stable and has more features than the old plugin. The old plugin is still available for use, but it is recommended to use the new plugin. When this option is checked, the new plugin is used. When it is unchecked, the old plugin is used. After changing this option, a restart of Scopy is required for the change to take effect.


OS:
        - any

Steps:
        * Step 1: Click on the connected Pluto device and connect to it. Select the Debugger plugin. By default, the IIO Explorer (Debugger v2) should be displayed.
                * Expected Result: The IIO Explorer (Debugger v2) should be displayed. If you notice the navigation bar with devices and attributes on the left side, the IIO Explorer is displayed.
        * Step 2: Go to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. Uncheck the "Use debugger V2 plugin" option.
                * Expected Result: A 'Restart' button should appear at the bottom of the preferences page. Click on the 'Restart' button. Scopy should restart.
        * Step 3: Click on the connected Pluto device and connect to it. Select the Debugger plugin.
                * Expected Result: The Debugger v1 plugin should be displayed. If you notice 2 sections, 'DEVICE SELECTION' and 'REGISTER MAP SETTINGS', the Debugger v1 plugin is displayed.
        * Step 4: Repeat step 2, but this time check the "Use debugger V2 plugin" option.
                * Expected Result: A 'Restart' button should appear at the bottom of the preferences page. Click on the 'Restart' button. Scopy should restart.
        * Step 5: Click on the connected Pluto device and connect to it. Select the Debugger plugin.
                * Expected Result: The IIO Explorer (Debugger v2) should be displayed. If you notice the navigation bar with devices and attributes on the left side, the IIO Explorer is displayed.

Test 2: Include debug attributes in IIO Explorer
----------------------------------------------------------------------------------------------------

UID:
        - TST.DBG.PREF.INCLUDE_DEBUG_ATTRIBUTES

Description:
        - This option allows you to include IIO debug attributes in the IIO Explorer. When this option is checked, debug attributes are included in the IIO Explorer. When it is unchecked, debug attributes are not included in the IIO Explorer. After changing this option, a restart of Scopy is required for the change to take effect.

Test prerequisites:
        - TST.DBG.EXPLR.NAV

OS:
        - any

Steps:
        * Step 1: Click on the connected Pluto device and connect to it. Select the Debugger plugin. By default, the IIO Explorer (Debugger v2) should be displayed.
        * Step 2: Go to the Preferences page (lower left corner, above the Analog Devices logo and the About button). From there, select the Debugger Plugin from the right-hand side menu. Check the "Include debug attributes in IIO Explorer" option. (if it is checked, leave it as it is)
        * Step 3: Go back to the Debugger plugin and notice the navigation bar on the left side. Double click on ad9361-phy and scroll down a bit.
                * Expected Result: Among the final device attributes of the ad9361-phy device, you should see the debug attributes. They should start with the 'adi,' prefix.
        * Step 4: Go back to the Preferences page and uncheck the "Include debug attributes in IIO Explorer" option.
        * Step 5: Restart Scopy.
        * Step 6: Click on the connected Pluto device and connect to it. Select the Debugger plugin.
        * Step 7: Look at the navigation bar on the left side. Double click on ad9361-phy and scroll down a bit.
                * Expected Result: The debug attributes should not be displayed among the final device attributes of the ad9361-phy device. (i.e., no attributes should start with the 'adi,' prefix)
