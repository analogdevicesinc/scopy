.. _swiot1l_tests:

SWIOT Plugin - Test Cases
=========================

The SWIOT plugin tests are a set of tests that are used to verify the functionality of the SWIOT plugin.
The tests are designed to be run in a specific order to ensure that the plugin is functioning correctly. 
The tests are divided into two main categories: **CONFIG** AND **RUNTIME**. 

The following apply for all the test cases in this suite.
If the test case has special requirements, they will be listed in the test case section.

Depends on:
    - Test TST.EMU.CONNECT
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with SWIOT plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - TST.SWIOT.COMPAT
-------------------------

Title:
    Test SWIOT plugin compatibility with device.

UID:
    TST.SWIOT.COMPAT

Description:
    This test verifies that the SWIOT plugin is compatible with the 
    selected device and that the plugin is able to correctly parse it. 

Steps:
    1. Open Scopy.
    2. Start the IIO-EMU process and connect to **swiot_config**.
    3. Open the SWIOT plugin - Config Instrument.
    4. Select the device from the device list.

Test 2 - TST.SWIOT.CONFIG
-------------------------

Title:
    Test SWIOT plugin configuration.

UID:
    TST.SWIOT.CONFIG

Description:
    This test verifies that the SWIOT plugin is able to configure the device
    correctly.

Steps:
    1. Open Scopy.
    2. Start the IIO-EMU process and connect to **swiot_config**.
    3. Open the SWIOT plugin - Config Instrument.
    4. Configure the device with the following settings:
        - **Setting 1**: Value 1
        - **Setting 2**: Value 2
        - **Setting 3**: Value 3
    



