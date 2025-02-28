.. _emu_tests:

EMU
===

.. note::

    User guide: :ref:`Scopy Overview <user_guide>`.

.. note::
    .. list-table:: 
       :widths: 50 30 30 50 50
       :header-rows: 1

       * - Tester
         - Test Date
         - Scopy version
         - Plugin version (N/A if not applicable)
         - Comments
       * - Alexandra Trifan
         - 13/02/2025
         - v2.0.0-beta-rc2-a3a7c43
         - N/A
         - OS: Ubuntu 22.04.5 LTS
       * - Alexandra Trifan
         - 14/02/2025
         - v2.0.0-beta-rc2-a3a7c43
         - N/A
         - OS: Debian 12 arm64
       * - Popa Andrei
         - 18.02.2025
         - 2.0.0 a3a7c43
         - N/A
         - Tested on Windows 10
       * - Danila Andrei
         - 19.02.2025
         - 2.0.0 a3a7c43
         - N/A
         - Tested on macOS 14.5

.. _tst-emu-en:

Test 1: Enable the emulator.
----------------------------

**UID:** TST.EMU.EN

**Description:** This test verifies if starting the emulator in Scopy works 
correctly.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
        - **Expected result:** The IIO device tab is selected, and the URI 
          "ip:127.0.0.1" is entered in the corresponding section. 
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 2: EMU is not installed.
-----------------------------

**UID:** TST.EMU.NOT

**Description:** This test verifies if Scopy handles the situation correctly 
when the IIO emulator is not found.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is not installed on the system.
    - Modify the "iio_emu_path" preference from "preferences.ini" file with an 
      invalid path (you can access the file through the **Open** button in the 
      general preferences).
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
        - **Expected result:** All the components are disabled and the message 
          "Can't find iio-emu in the system!" is displayed. 
        - **Actual result:** Emulator is still available (Windows 10, macOS 14.5)

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:** Windows 10: With the iio-emu not installed in system and a bad preferences path, emulator is still found as long as iio-emu.exe is in the same folder as scopy.exe; macOS 14.5: Unable to locate the preferences.ini

..
  Any comments about the test goes here.

**Result:** FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 3: Disable the emulator.
-----------------------------

**UID:** TST.EMU.DIS

**Description:** This test verifies if stopping the emulator in Scopy works 
correctly.

**Test prerequisites:**
    - :ref:`TST.EMU.EN <tst-emu-en>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
    6. Click on **Emulator** tab.
    7. Click on **Disable** button.
        - **Expected result:** The fields in this tab become editable, and the 
          **Enable** button appears. 
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 4: Disable the emulator while the emulated device is connected.
--------------------------------------------------------------------

**UID:** TST.EMU.DIS_CONN

**Description:** The test verifies if Scopy handles the situation correctly 
when an emulated device is connected and the "iio-emu" process is stopped.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Start the iio-emu process using "adalm2000" in the dropdown and 
       "ip:127.0.0.1" as URI.
    3. Add the device in device browser.
    4. Connect to emu device.
    5. Click on **Add device** (+) button.
    6. Click on **Emulator** tab.
    7. Click on **Disable** button.
        - **Expected result:** The fields in this tab become editable, and the 
          **Enable** button appears. After a few seconds, the device is 
          disconnected.
        - **Actual result:** The device is not disconnected on Debian on Raspberry Pi 4 but 
          is disconnected on Ubuntu x86_64.

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:** The iio-emu is disabled and the device is disconnected as expected on Ubuntu 22.04.5 LTS. However, on Debian 12 arm64, 
the device is not disconnected - due to iio-emu process not properly stopped.

..
  Any comments about the test goes here.

**Result:** FAIL

..
  The result of the test goes here (PASS/FAIL).


Test 5: Verify adalm2000 emu device.
------------------------------------

**UID:** TST.EMU.EN_ADALM2000

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using adalm2000 device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "adalm2000".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible 
          plugins are: M2kPlugin (checked), ADCPlugin, DACPlugin, DataLoggerPlugin, 
          DebuggerPlugin, RegmapPlugin.
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 6: Verify pluto emu device.
--------------------------------

**UID:** TST.EMU.EN_PLUTO

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using pluto device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "pluto".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible 
          plugins are: ADCPlugin (checked), DACPlugin (checked), DataLoggerPlugin 
          (checked), DebuggerPlugin (checked), RegmapPlugin (checked). 
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 7: Verify pqm emu device.
------------------------------

**UID:** TST.EMU.EN_PQM

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using pqm device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "pqm".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible 
          plugins are: PQMPlugin (checked), ADCPlugin, DACPlugin, DataLoggerPlugin, 
          DebuggerPlugin.
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 8: Verify swiot_config emu device.
---------------------------------------

**UID:** TST.EMU.EN_SWIOT_CONFIG

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using swiot_config device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "swiot_config".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible 
          plugins are: SWIOTPlugin (checked), DataLoggerPlugin, DebuggerPlugin.
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 9: Verify swiot_runtime emu device.
----------------------------------------

**UID:** TST.EMU.EN_SWIOT_RUNTIME

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using swiot_runtime device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "swiot_runtime".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The compatible plugins page opens. The compatible 
          plugins are: SWIOTPlugin (checked), ADCPlugin, DataLoggerPlugin, 
          DebuggerPlugin, RegmapPlugin.
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).


Test 10: Verify generic emu device.
-----------------------------------

**UID:** TST.EMU.EN_GENERIC

**Description:** This test verifies if starting the emulator in Scopy works 
correctly (using generic device).

**Test prerequisites:**
    - :ref:`TST.ADD.VERIFY <tst-add-verify>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Open Scopy.
    2. Click on **Add device** (+) button.
    3. Click on **Emulator** tab.
    4. From "Device" dropdown select "generic".
    5. Click on **Enable** button.
    6. Click on **Verify** button.
        - **Expected result:** The message **"ip:127.0.0.1" not a valid 
          context!** is displayed. 
        - **Actual result:** As expected

..
  Actual test result goes here.
..

**Tested OS:** Ubuntu 22.04.5 LTS, Debian 12 arm64, Windows 10, macOS 14.5 

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS

..
  The result of the test goes here (PASS/FAIL).
