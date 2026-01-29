.. _imuanalyzer_tests:

IMU Analyzer Test Suite
================================================================================

.. note::

  User guide :ref:`IMU Analyzer user guide <imuanalyzer>`

The following apply to all tests below, any special requirements will be mentioned.

**Prerequisites:**
    - Scopy v2.2.0 or later with IMU Analyzer plugin installed on the system
    - OpenGL-capable graphics hardware for 3D visualization
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

Setup Environment
------------------

.. _imuanalyzer_adis_device_setup:

**Compatible ADIS IMU Device:**
    - Open Scopy
    - Connect to a device with ADIS IMU sensor (ADIS16550, ADIS16545, ADIS16547, etc.)
    - Ensure device appears in IIO context with "adis" in device name

.. _imuanalyzer_emulated_device_setup:

**Emulated ADIS Device:**
    - Open Scopy
    - Start iio-emu process with ADIS device emulation
    - Connect to emulated ADIS device for testing without physical hardware

.. _imuanalyzer_incompatible_device_setup:

**Incompatible Device:**
    - Open Scopy
    - Connect to a device without ADIS IMU sensor (e.g., ADALM-PLUTO)
    - Used for testing plugin compatibility detection

.. _TST_IMUANALYZER_PLUGIN_DETECTION:

Test 1: Plugin Detection and ADIS Compatibility
------------------------------------------------

**UID:** TST.IMUANALYZER.PLUGIN_DETECTION

**RBP:** P0

**Description:** Verify that the IMU Analyzer plugin is detected and only activates for compatible ADIS devices

**OS:** ANY

**Preconditions:**
    - IMU Analyzer plugin is installed
    - OpenGL support is available

**Steps:**
    1. Connect to :ref:`Compatible ADIS IMU Device <imuanalyzer_adis_device_setup>`
        - **Expected result:** IMU Analyzer plugin is automatically detected and available in tool list
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Verify plugin appears in the device tool list
        - **Expected result:** "IMUAnalyzer" tool is listed and enabled
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Disconnect and connect to :ref:`Incompatible Device <imuanalyzer_incompatible_device_setup>`
        - **Expected result:** IMU Analyzer plugin does not appear or is marked as incompatible
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


.. _TST_IMUANALYZER_3D_VISUALIZATION:

Test 2: 3D Visualization and Scene Rendering
---------------------------------------------

**UID:** TST.IMUANALYZER.3D_VISUALIZATION

**RBP:** P1

**Description:** Verify that 3D visualization engine works correctly and displays IMU orientation in real-time

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible ADIS IMU Device <imuanalyzer_adis_device_setup>`
    - OpenGL drivers are properly installed

**Steps:**
    1. Open IMU Analyzer tool
        - **Expected result:**
            - Tool opens with tabbed interface
            - "3D View" tab is active and visible
            - 3D scene is rendered without errors
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Click **Run** button to start data acquisition
        - **Expected result:**
            - 3D object appears in scene representing IMU orientation
            - Real-time updates begin 
            - No rendering errors or black screen
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Physically rotate or move the IMU device (if possible)
        - **Expected result:**
            - 3D visualization updates in real-time to match physical orientation
            - Smooth animation with no lag or stuttering
            - Axes are clearly visible and color-coded
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


.. _TST_IMUANALYZER_VIEW_CONTROLS:

Test 3: Interactive 3D View Controls
-------------------------------------

**UID:** TST.IMUANALYZER.VIEW_CONTROLS

**RBP:** P2

**Description:** Verify that 3D view controls work correctly for navigation and interaction

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible ADIS IMU Device <imuanalyzer_adis_device_setup>`

**Prerequisites:**
    - :ref:`3D Visualization and Scene Rendering <TST_IMUANALYZER_3D_VISUALIZATION>` is passed

**Steps:**
    1. Test mouse navigation controls in 3D view
        - **Expected result:**
            - Left mouse drag rotates the view around the 3D object
            - Mouse scroll wheel zooms in and out
            - View changes are smooth and responsive
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Click **Reset View** button
        - **Expected result:**
            - View returns to default angle and zoom level
            - Reset is immediate and smooth
            - 3D object remains properly oriented
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Test view manipulation while data is running
        - **Expected result:**
            - View controls work normally during real-time updates
            - No interference between view manipulation and data updates
            - Performance remains smooth
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


.. _TST_IMUANALYZER_MEASUREMENT_MODE:

Test 4: Measurement Mode and Data Visualization
------------------------------------------------

**UID:** TST.IMUANALYZER.MEASUREMENT_MODE

**RBP:** P2

**Description:** Verify that measurement mode provides accurate sensor data display and analysis

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible ADIS IMU Device <imuanalyzer_adis_device_setup>`

**Prerequisites:**
    - :ref:`3D Visualization and Scene Rendering <TST_IMUANALYZER_3D_VISUALIZATION>` is passed

**Steps:**
    1. Enable measurement mode
        - **Expected result:**
            - **Measure** button toggles to active state
            - Data visualization panel appears
            - Numerical data displays with real-time updates
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Verify sensor data display
        - **Expected result:**
            - Rotation data (pitch, roll, yaw) displayed with appropriate units
            - Position data calculated from acceleration integration
            - Temperature reading shows current sensor temperature
            - All values update in real-time
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Toggle measurement mode off and on
        - **Expected result:**
            - Panel hides when measurement mode disabled
            - Panel reappears when re-enabled
            - Data continues updating correctly
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
