.. _profile_generator_tests:

ADRV9002 Profile Generator - Test Suite
=======================================

.. note::

   User guide: :ref:`ADRV9002 user guide<adrv9002>`

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


**Prerequisites:**
    - Scopy v2.1.0 or later with ADRV9002 plugin installed
    - ADRV9002 device connected
    - Device drivers and firmware are up to date
    - Reset .ini files to default by pressing **Reset** button in Preferences
    - CLI tool libadrv9002-iio installed and accessible


Setup Environment
-----------------

.. _profile_generator_setup:

**ADRV9002.profile_generator:**
   - libadrv9002-iio external tool must be installed and accessible
   - ADRV9002 device connected and detected in Scopy
   - System environment properly configured for external tool execution
   - Appropriate permissions for file I/O operations

.. _profile_generator_no_cli:

**ADRV9002.profile_no_cli:**
   - ADRV9002 device connected but adrv9002-iio-cli NOT in PATH
   - Profile Generator tab accessible but CLI functionality disabled
   - Use for testing CLI unavailable scenarios


Profile Generator CLI Tests
---------------------------

Test 1: CLI Not Available
~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.CLI_NOT_AVAILABLE

**RBP:** P3

**Description:** Verify profile generator behavior when CLI tool is not available.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_no_cli <profile_generator_no_cli>` setup

**Steps:**
   1. Open Profile Generator tab
   2. Verify action buttons are disabled

    - **Expected result:** UI elements are disabled download cli button shows 
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Check status message about CLI availability

    - **Expected result:** Clear message indicating CLI tool not found
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 2: CLI Available but Not Compatible
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.CLI_NOT_COMPATIBLE

**RBP:** P3

**Description:** Verify profile generator behavior when CLI tool is available but not compatible version.

**OS:** ANY

**Preconditions:**
   - ADRV9002 device connected
   - adrv9002-iio-cli is installed but incompatible version
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup

**Steps:**
   1. Open Profile Generator tab
   2. Check warning message display

    - **Expected result:** Warning message indicates CLI version incompatibility
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify download CLI button visibility

    - **Expected result:** Download CLI button is visible and functional
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Test action button states

    - **Expected result:** UI buttons remain enabled despite compatibility warning
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

.. _TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE:

Test 3: CLI Available
~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE

**RBP:** P3

**Description:** Verify profile generator behavior when CLI tool is available.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - Ensure adrv9002-iio-cli is installed and in system PATH

**Steps:**
   1. Open Profile Generator tab
   2. Check CLI availability status display

    - **Expected result:** Status shows CLI version and availability
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify all action buttons are enabled

    - **Expected result:** Save Profile, Save Stream, Load to Device buttons are enabled
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


LTE Configuration Tests
-----------------------

Test 4: LTE Default Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.LTE_DEFAULT_CONFIG

**RBP:** P3

**Description:** Test LTE preset configuration application and constraints.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - CLI available :ref:`Test 3<TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE>`

**Steps:**
   1. Open Profile Generator tab
   2. Select "LTE" from preset dropdown
   3. Verify radio configuration defaults

    - **Expected result:** FDD mode, LVDS interface, 4 lanes, appropriate clock settings
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Verify channel configuration constraints

    - **Expected result:** Channels enabled with LTE-appropriate bandwidths and sample rates
    - **Actual result:**

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 5: LTE Configuration Modification
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.LTE_CONFIG_MODIFY

**RBP:** P3

**Description:** Test modification of LTE configuration parameters.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - CLI available :ref:`Test 3<TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE>`
   - LTE preset is selected

**Steps:**
   1. Open Profile Generator tab
   2. Change duplex mode from FDD to TDD
   3. Modify RX1 channel bandwidth

    - **Expected result:** Bandwidth changes, sample rate updates accordingly
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Live Device Configuration Tests
-------------------------------

Test 6: Live Device Configuration Read
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.LIVE_DEVICE_READ

**RBP:** P3

**Description:** Test reading current device configuration into profile generator.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - CLI available :ref:`Test 3<TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE>`

**Steps:**
   1. Select "Live Device" from preset dropdown
   2. Verify channel states match device

    - **Expected result:** Enabled/disabled channels match device state
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify frequency and bandwidth settings

    - **Expected result:** Values match current device configuration
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Check debug info display

    - **Expected result:** Debug info shows device-sourced data
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


File Generation Tests
---------------------

Test 7: Stream File Generation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.PROFILE_FILE_GENERATION

**RBP:** P3

**Description:** Test generation of profile configuration file.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - CLI available :ref:`Test 3<TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE>`
   - Valid configuration is set (LTE or custom)

**Steps:**
   1. Open Profile Generator tab
   2. Configure channels and radio settings
   3. Click "Save Stream to File" button

    - **Expected result:** File dialog opens for stream file selection
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Specify output file path and save

    - **Expected result:** CLI executes successfully, stream file is generated
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 8: Profile File Generation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.PROFILE_FILE_GENERATION

**RBP:** P3

**Description:** Test generation of profile configuration file.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - Test 3 (CLI Available) must pass
   - Valid configuration is set (LTE or custom)

**Steps:**
   1. Open Profile Generator tab
   2. Configure channels and radio settings
   3. Click "Save Profile to File" button

    - **Expected result:** File dialog opens for profile file selection
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Specify output file path and save

    - **Expected result:** CLI executes successfully, profile file is generated
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Verify generated file contents

    - **Expected result:** Profile file contains valid configuration data
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..


Test 9: Load to Device
~~~~~~~~~~~~~~~~~~~~~~~~

**UID:** TST.ADRV9002.PROFILE_GEN.LOAD_TO_DEVICE

**RBP:** P3

**Description:** Test loading generated profile to connected device.

**OS:** ANY

**Preconditions:**
   - Use :ref:`ADRV9002.profile_generator <profile_generator_setup>` setup
   - CLI available :ref:`Test 3<TST.ADRV9002.PROFILE_GEN.CLI_AVAILABLE>`
   - Valid configuration is set (LTE or custom)

**Steps:**
   1. Open Profile Generator tab
   2. Configure channels and radio settings

    - **Expected result:** Configuration is valid and complete
    - **Actual result:**

..
  Actual test result goes here.
..

   2. Click "Load to Device" button

    - **Expected result:** CLI generates profile and stream files
    - **Actual result:**

..
  Actual test result goes here.
..

   3. Verify profile loading progress

    - **Expected result:** Progress indicators show profile and stream loading steps
    - **Actual result:**

..
  Actual test result goes here.
..

   4. Check device configuration after loading

    - **Expected result:** Device operates with new configuration, Controls tab reflects changes
    - **Actual result:**

..
  Actual test result goes here.
..

**Tested OS:**

..
  Details about the tested OS goes here.
..

**Comments:**

..
  Any comments about the test goes here.
..

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).
..

