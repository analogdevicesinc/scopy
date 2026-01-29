.. _rfpowermeter_tests:

RF Power Meter Test Suite
================================================================================

.. note::

  User guide :ref:`RF Power Meter user guide <rfpowermeter>`

The following apply to all tests below, any special requirements will be mentioned.

**Prerequisites:**
    - Scopy v2.2.0 or later with RF Power Meter plugin installed on the system
    - Data Logger plugin must be installed and enabled
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

.. _rfpowermeter_compatible_device_setup:

**Compatible RF Power Meter Device:**
    - Open Scopy
    - Connect to a device with ``powrms`` IIO interface (RF power measurement device)
    - Ensure Data Logger plugin is enabled in plugin manager

.. _rfpowermeter_incompatible_device_setup:

**Incompatible Device:**
    - Open Scopy
    - Connect to a device without ``powrms`` IIO interface (e.g., ADALM-PLUTO)
    - Used for testing plugin compatibility detection

.. _TST_RFPOWERMETER_PLUGIN_DETECTION:

Test 1: Plugin Detection and Compatibility
--------------------------------------------

**UID:** TST.RFPOWERMETER.PLUGIN_DETECTION

**RBP:** P0

**Description:** Verify that the RF Power Meter plugin is detected and only activates for compatible devices

**OS:** ANY

**Preconditions:**
    - RF Power Meter plugin is installed
    - Data Logger plugin is enabled

**Steps:**
    1. Connect to :ref:`Compatible RF Power Meter Device <rfpowermeter_compatible_device_setup>`
        - **Expected result:** RF Power Meter plugin is automatically detected and available
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Verify plugin appears in the device plugin list
        - **Expected result:** RF Power Meter plugin is listed as active/compatible
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Disconnect and connect to :ref:`Incompatible Device <rfpowermeter_incompatible_device_setup>`
        - **Expected result:** RF Power Meter plugin does not appear or is marked as incompatible
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


.. _TST_RFPOWERMETER_DATALOGGER_DEPENDENCY:

Test 2: Data Logger Plugin Dependency
---------------------------------------

**UID:** TST.RFPOWERMETER.DATALOGGER_DEPENDENCY

**RBP:** P1

**Description:** Verify that RF Power Meter correctly handles Data Logger plugin dependency

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible RF Power Meter Device <rfpowermeter_compatible_device_setup>`

**Steps:**
    1. Disable Data Logger plugin in plugin manager
    2. Attempt to connect to RF Power Meter device
        - **Expected result:**
            - Status bar message appears: "RF Power Meter requires DataLogger plugin to be enabled"
            - RF Power Meter plugin connection fails gracefully
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Enable Data Logger plugin
    4. Reconnect to RF Power Meter device
        - **Expected result:** RF Power Meter plugin connects successfully
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


.. _TST_RFPOWERMETER_AUTOMATIC_CONFIGURATION:

Test 3: Automatic Data Logger Configuration
---------------------------------------------

**UID:** TST.RFPOWERMETER.AUTOMATIC_CONFIGURATION

**RBP:** P2

**Description:** Verify that RF Power Meter automatically configures Data Logger with correct settings

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible RF Power Meter Device <rfpowermeter_compatible_device_setup>`

**Prerequisites:**
    - :ref:`Data Logger Plugin Dependency <TST_RFPOWERMETER_DATALOGGER_DEPENDENCY>` is passed

**Steps:**
    1. Open Data Logger tool after RF Power Meter connection
        - **Expected result:**
            - Tool name is changed from "Data Logger" to "RF Power Meter"
            - Display mode is set to seven-segment display
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Verify monitor configuration
        - **Expected result:**
            - "Power Forward" (powrms:power5) monitor is enabled and configured with dBm units
            - "Power Reverse" (powrms:power6) monitor is enabled and configured with dBm units
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


.. _TST_RFPOWERMETER_FREQUENCY_CONTROL:

Test 4: Frequency Control Widget
----------------------------------

**UID:** TST.RFPOWERMETER.FREQUENCY_CONTROL

**RBP:** P3

**Description:** Verify that frequency control widget is properly integrated and functional

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`Compatible RF Power Meter Device <rfpowermeter_compatible_device_setup>`

**Prerequisites:**
    - :ref:`Automatic Data Logger Configuration <TST_RFPOWERMETER_AUTOMATIC_CONFIGURATION>` is passed

**Steps:**
    1. Locate frequency control widget in RF Power Meter interface
        - **Expected result:** Frequency control widget is visible in the seven-segment display area
        - **Actual result:**

..
  Actual test result goes here.
..

    2. Modify frequency setting
        - **Expected result:**
            - Frequency value can be changed
            - Changes are applied to the device
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

