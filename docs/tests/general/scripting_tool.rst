.. _scripting_tool_tests:


Scripting Tool - Test Suite
============================================================================================================================


The following apply to all tests below, any special requirements will be mentioned.

**Prerequisites:**
    - Scopy v2.1.0 or later with Scripting installed on the system
    - Tests listed as dependencies are successfully completed
    - Reset .ini files to default by pressing **Reset** button in Preferences

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


Test 1: Enabling the scripting tool from Preferences
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.ENABLE

**Description**: This test verifies that the scripting tool can be enabled from the Preferences menu.

**Preconditions**:
        - Scopy is installed on the system
        - OS: ANY

**Steps**:
        1. Open the Scopy application
        2. Go to Preferences → Generic
        3. Enable the "Scripting" option
                - **Expected Result:** The scripting tool becomes available in the main interface (appears in the tool menu )
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

Test 2: Scripting tool loads successfully in Scopy
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.LOAD

**Description**: This test verifies that the scripting tool loads and is accessible in the Scopy interface after being enabled.

**Preconditions**:
        - Scripting tool is enabled in Preferences
        - Scopy is installed on the system
        - OS: ANY

**Steps**:
        1. Open the Scopy application
        2. Confirm that the scripting tool appears in the tool menu or sidebar
                - **Expected Result:** Scripting tool is visible and can be selected
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

Test 3: User can run a script
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.RUN

**Description**: This test verifies that a user can execute a script using the scripting tool, including connecting to a Pluto device in emulation mode.

**Preconditions**:
        - Scripting tool is loaded
        - Pluto emulation is enabled
        - A valid script is available
        - OS: ANY

**Steps**:
        1. Enable Pluto emulation in Scopy.
        2. Open the scripting tool.
        3. Enter the following script in the editor:

            .. code-block:: javascript

                var deviceID = scopy.addDevice("ip:127.0.0.1")
                scopy.connectDevice(deviceID)

        4. Click the "Run" button
                - **Expected Result:** The script executes, connects to the emulated Pluto device, and produces output/results
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

Test 4: User can save a script
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.SAVE

**Description**: This test verifies that a user can save a script from the scripting tool.

**Preconditions**:
        - Scripting tool is loaded
        - A script is present in the editor
        - OS: ANY

**Steps**:
        1. Open the scripting tool
        2. Enter or edit a script
        3. Click the "Save" button
                - **Expected Result:** The script is saved to disk and can be found in the chosen location
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

Test 5: User can load a script
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.LOADFILE

**Description**: This test verifies that a user can load an existing script into the scripting tool.

**Preconditions**:
        - Scripting tool is loaded
        - A script file is available on disk
        - OS: ANY

**Steps**:
        1. Open the scripting tool
        2. Click the "Load" button
        3. Select a script file from disk
                - **Expected Result:** The script is loaded into the editor and can be edited or run
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

Test 6: Output/results are displayed in the scripting tool's output panel
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.OUTPUT

**Description**: This test verifies that the output/results of a script are displayed in the scripting tool's output panel.

**Preconditions**:
        - Scripting tool is loaded
        - A script is executed
        - OS: ANY

**Steps**:
        1. Run a script in the scripting tool
        2. Observe the output panel
                - **Expected Result:** The output/results of the script are displayed in the output panel
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

Test 7: Integration with other Scopy plugins
----------------------------------------------------------------------------------------------------------------------------

**UID**: TST.SCRIPT.INTEGRATION

**Description**: This test verifies that the scripting tool can interact with other Scopy plugins (e.g., access device data, control plugin features) by switching tools and modifying register values.

**Preconditions**:
        - Scripting tool is loaded
        - Other Scopy plugins are enabled and available
        - Physical Pluto device is connected
        - OS: ANY

**Steps**:
        1. Connect a physical Pluto device to the system.
        2. Open the scripting tool.
        3. Enter the following script in the editor:

            .. code-block:: javascript

                scopy.switchTool("Data Logger ")
                msleep(1000)
                scopy.switchTool("Register map")
                regmap.write("0x00", "0x01")
                msleep(1000)
                regmap.write("0x00", "0x00")

        4. Run the script
                - **Expected Result:** The script switches to Data Logger, then Register Map, modifies the first register value, and restores it. The plugins respond as expected and the register value changes are reflected.
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
