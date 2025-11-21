.. _pqm_tests:

PQM Tests
=========

Rms tool
--------

.. note::

    User guide: :ref:`Rms <pqm-rms>`.


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
^^^^^^^^^^^^^^^^^^

.. _pqm-emu-rms:

**PQM.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "pqm" in the dropdown and "ip:127.0.0.1" 
      as URI.
    - Run: `python3 pqmDataWriter.py <https://github.com/analogdevicesinc/scopy/blob/main/plugins/pqm/res/pqmDataWriter.py>`_.
    - Add the device in device browser.
    - Connect to "ip:127.0.0.1".

Test 1: Open Rms tool.
^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_OPEN

**RBP:** P3

**Description:** This test verifies that the Rms tool opens correctly and is 
accessible without errors.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Observe the application's behavior.
        - **Expected result:** In the top section of the tool widget, there is 
          an Info button, a PQEvents indicator, a Run button, a Single button, 
          and a Settings button. The Settings menu is open, and the Settings 
          button is in a checked state. In the center of the widget, there are 
          two plots, each with labels positioned above them.
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
 

Test 2: Info button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_INFO

**RBP:** P3

**Description:** This test verifies if the Info button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Click on the **Info** button.
        - **Expected result:** The RMS documentation page is opened in the 
          browser. 
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


Test 3: Single acquisition.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_SINGLE

**RBP:** P3

**Description:** This test verifies if the single acquisition works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Click on the **Single** button.
        - **Expected result:** Both the plot and the header labels are populated 
          with values. A single block of data is acquired.
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


.. _tst-pqm-rms-run:

Test 4: Data acquisition.
^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_RUN

**RBP:** P3

**Description:** This test verifies if the data acquisition works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Click on the **Start** button.
        - **Expected result:** Both the plot and the header labels are 
          populated with values. The data is updated every few seconds.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the **Stop** button.
        - **Expected result:** The data acquisition stops.
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


Test 5: Settings button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_SETTINGS

**RBP:** P3

**Description:** This test verifies if the Settings button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Click on the **Settings** button.
        - **Expected result:** The settings menu collapses and the button 
          becomes unchecked. 
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


Test 6: Data logging.
^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_LOG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.RMS_RUN <tst-pqm-rms-run>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Make sure that the settings menu is open.
    3. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    4. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.    
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Click on the **Browse** button.
        - **Expected result:** A explorer window is displayed. Only directories 
          can be selected.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Select a directory and click on the **Open** button.
        - **Expected result:** The explorer window is closed and the path to 
          that directory is introduced in the browser section.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click on the **Run** button.
        - **Expected result:** The **LOG** section remains open, but its 
          elements can no longer be accessed and a .csv file is created in 
          the selected directory.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Click on the **Stop** button.
        - **Expected result:** The **LOG** section can now be accessed, 
          and the .csv file is populated with data.
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


Test 7: Data logging wrong path.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_LOG_WRONG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Make sure that the settings menu is open.
    3. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    4. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.    
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Enter an invalid path in the line edit.
    6. Click on the **Run** button.
        - **Expected result:** The **LOG** section get closed and no .csv 
          file is created.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click on the **Stop** button.
        - **Expected result:** The **LOG** section remains closed.
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


Test 8: PQEvents.
^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.RMS_PQEVENTS

**RBP:** P3

**Description:** This test verifies if the PQEvents indicator works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.RMS_RUN <tst-pqm-rms-run>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-rms>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Rms** tool.
    2. Click on the **Run** button.
    3. Run ``iio_attr -u ip:127.0.0.1 -c pqm count0 countEvent 1`` command.
        - **Expected result:**  The PQEvents indicator becomes active.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Stop** button.
        - **Expected result:**  The PQEvents indicator is still active. 
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Click on the **PQEvents** indicator.
        - **Expected result:**  The PQEvents indicator becomes inactive. 
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Run ``iio_attr -u ip:127.0.0.1 -c pqm count0 countEvent 0`` command.

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Harmonics tool
--------------

.. note::

    User guide: :ref:`Harmonics <pqm-harmonics>`.


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
^^^^^^^^^^^^^^^^^^

.. _pqm-emu-harmonics:

**PQM.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "pqm" in the dropdown and "ip:127.0.0.1" 
      as URI.
    - Run: `python3 pqmDataWriter.py <https://github.com/analogdevicesinc/scopy/blob/main/plugins/pqm/res/pqmDataWriter.py>`_.
    - Add the device in device browser.
    - Connect to "ip:127.0.0.1".

Test 1: Open Harmonics tool.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_OPEN

**RBP:** P3

**Description:** This test verifies that the Harmonics tool opens correctly 
and is accessible without errors.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Observe the application's behavior.
        - **Expected result:** In the top section of the tool widget, there 
          is an Info button, a PQEvents indicator, a Run button, a Single button, 
          and a Settings button. The Settings menu is open, and the Settings 
          button is in a checked state. In the center, there are three 
          components: THD, a table, and a plot.
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


Test 2: Info button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_INFO

**RBP:** P3

**Description:** This test verifies if the Info button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Click on the **Info** button.
        - **Expected result:** The Harmonics documentation page is opened in the 
          browser. 
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


.. _tst-pqm-harmonics-single:

Test 3: Single acquisition.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_SINGLE

**RBP:** P3

**Description:** This test verifies if the single acquisition works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Click on the **Single** button.
        - **Expected result:** All the components are populated with values. The 
          data in the table must be reflected on the plot. A single block of data is 
          acquired.
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


.. _tst-pqm-harmonics-run:

Test 4: Data acquisition.
^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_RUN

**RBP:** P3

**Description:** This test verifies if the data acquisition works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Click on the **Start** button.
        - **Expected result:** The data is updated every few seconds (only the 
          values from the table and from the plot are updated). The data in the table must 
          be reflected on the plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the **Stop** button.
        - **Expected result:** The data acquisition stops.
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


Test 5: The features of the table.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_TABLE

**RBP:** P3

**Description:** This test verifies if the table functionalities works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.HARMONICS_SINGLE <tst-pqm-harmonics-single>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Click on the **Single** button.
        - **Expected result:** The row Ia is highlighted and the corresponding 
          values are drawn on the plot.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Make sure that the settings menu is visible.
    4. Click on the **Ib** row from the table.
        - **Expected result:** The entire row is selected and the Ib values are 
          displayed on the plot. In the setting menu, the active channel is also Ib.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Select the first 6 values from row **Ic** (click on the first value and hold it down until you reach column 5).
        - **Expected result:** Only the respective values remain highlighted, 
          and only the selected columns appear on the plot. The drawing color 
          is another one. 
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


Test 6: Settings button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_SETTINGS

**RBP:** P3

**Description:** This test verifies if the Settings button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Harmonics** tool.
    2. Click on the **Settings** button.
        - **Expected result:** The settings menu collapses and the button 
          becomes unchecked. 
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the **Settings** button.
        - **Expected result:** The settings menu opens, and the button is 
          checked. The menu contains two sections: **General** and **Log**. 
          The **General** section contains **Harmonics type** (set to 
          *harmonics* by default) and **Active channel** (set to *Ia* by 
          default).
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Select **Ib** from active channels dropdown.
        - **Expected result:** The Ib row from the table is highlighted (if the 
          table is populated, then the plot data is changed accordingly). 
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


Test 7: Data logging.
^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_LOG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.HARMONICS_RUN <tst-pqm-harmonics-run>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Make sure that the settings menu is open.
    2. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    3. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.  
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Browse** button.
        - **Expected result:** An explorer window is displayed. Only directories 
          can be selected.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Select a directory and click on the **Open** button.
        - **Expected result:** The explorer window is closed and the path to that 
          directory is introduced in the browse section.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the **Run** button.
        - **Expected result:** The **LOG** section remains open, but its elements 
          can no longer be accessed and a .csv file is created in the selected directory.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click on the **Stop** button.
        - **Expected result:** The **LOG** section can now be accessed, and the 
          .csv file is populated with data.
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


Test 8: Data logging wrong path.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_LOG_WRONG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Make sure that the settings menu is open.
    2. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    3. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.    
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Enter an invalid path in the line edit.
    5. Click on the **Run** button.
        - **Expected result:** The **LOG** section is closed and no .csv file is 
          created.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the **Stop** button.
        - **Expected result:** The **LOG** section remains closed.
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


Test 9: PQEvents.
^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.HARMONICS_PQEVENTS

**RBP:** P3

**Description:** This test verifies if the PQEvents indicator works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.HARMONICS_RUN <tst-pqm-harmonics-run>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-harmonics>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Run** button.
    2. Run ``iio_attr -u ip:127.0.0.1 -c pqm count0 countEvent 1`` command.
        - **Expected result:**  The PQEvents indicator becomes active.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the **Stop** button.
        - **Expected result:**  The PQEvents indicator is still active. 
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **PQEvents** indicator.
        - **Expected result:**  The PQEvents indicator becomes inactive. 
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Run ``iio_attr -u ip:127.0.0.1 -c pqm count0 countEvent 0`` command.

**Tested OS:**

..
  Details about the tested OS goes here.

**Comments:**

..
  Any comments about the test goes here.

**Result:** PASS/FAIL

..
  The result of the test goes here (PASS/FAIL).


Waveform tool
-------------

.. note::

    User guide: :ref:`Waveform <pqm-waveform>`.


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
^^^^^^^^^^^^^^^^^^

.. _pqm-emu-waveform:

**PQM.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "pqm" in the dropdown and "ip:127.0.0.1" 
      as URI.
    - Add the device in device browser.
    - Connect to "ip:127.0.0.1".

Test 1: Open Waveform tool.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_OPEN

**RBP:** P3

**Description:** This test verifies that the Waveform tool opens correctly and 
is accessible without errors.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Observe the application's behavior.
        - **Expected result:** In the top section of the tool widget, there is 
          an Info button, a Run button, a Single button, and a Settings button. 
          The Settings menu is open, and the Settings button is in a checked 
          state. In the center, there are two plots.
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


Test 2: Info button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_INFO

**RBP:** P3

**Description:** This test verifies if the Info button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Click on the **Info** button.
        - **Expected result:** The Waveform documentation page is opened in the 
          browser. 
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


Test 3: Settings button check.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_SETTINGS

**RBP:** P3

**Description:** This test verifies if the Settings button works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Click on the **Settings** button.
        - **Expected result:** The settings menu collapses and the button 
          becomes unchecked. 
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click on the **Settings** button.
        - **Expected result:** The settings menu opens, and the button is 
          checked.
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


Test 4: Settings timespan check.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_SETTINGS_TIMESPAN

**RBP:** P3

**Description:** This test verifies if the timespan spin box works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the settings menu is open.
    3. Click on (+) button. 
        - **Expected result:** The timespan is increased and the x-axis range is 
          now  [-newValue, 0]. The maximum value must be 10. 
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on (-) button. 
        - **Expected result:** The timespan is decreased and the x-axis range is 
          now  [-newValue, 0]. The minimum value must be 0.02. 
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


Test 5: Settings rolling switch check.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_SETTINGS_ROLLING

**RBP:** P3

**Description:** This test verifies if the rolling mode switch works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the settings menu is open.
    3. Enable **Rolling mode** swich.
        - **Expected result:** The switch is turned on, and the "triggered by" 
          dropdown is inactive.
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


Test 6: Single acquisition triggered.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_SINGLE_TRIG

**RBP:** P3

**Description:** This test verifies if the single acquisition works correctly 
(triggered mode).

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the **Rolling mode** switch is turned off.
    3. Click on the **Single** button.
        - **Expected result:** The curves are displayed on the plot in single 
          shot format. A single block of data is acquired.
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


Test 7: Single acquisition rolling.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_SINGLE_ROLL

**RBP:** P3

**Description:** This test verifies if the single acquisition works correctly 
(rolling mode).

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the **Rolling mode** switch is turned on.
    3. Click on the **Single** button.
        - **Expected result:** The curves are drawn from right to left until all 
          the required samples for the plot are acquired. A single block of data is 
          acquired.
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


Test 8: Data acquisition triggered.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_RUN_TRIG

**RBP:** P3

**Description:** This test verifies if the data acquisition works correctly 
(triggered mode).

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the **Rolling mode** switch is turned off.
    3. Click on the **Start** button.
        - **Expected result:** The curves are displayed on the plot in single 
          shot format. When new data is available, it is displayed on the screen.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Stop** button.
        - **Expected result:** The data acquisition stops.
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


.. _tst-pqm-waveform-run-roll:

Test 9: Data acquisition rolling.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_RUN_ROLL

**RBP:** P3

**Description:** This test verifies if the data acquisition works correctly 
(rolling mode).

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Make sure that the **Rolling mode** switch is turned on.
    3. Click on the **Start** button.
        - **Expected result:** The curves are drawn from right to left until all 
          the required samples for the plot are acquired. If there are non-zero values, a 
          continuous motion illusion is created.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Stop** button.
        - **Expected result:** The data acquisition stops.
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


Test 10: Plot zoom.
^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_PLOT_ZOOM

**RBP:** P3

**Description:** This test verifies if, when zooming in on one plot, the x-axis 
range of the other plot is modified and matches the zoom range.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Waveform** tool.
    2. Zoom in on the first plot to a range smaller than the x-axis range.
        - **Expected result:** The x-axis range of the second plot is modified 
          and matches the range of the first plot.
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


Test 11: Data logging.
^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_LOG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.PQM.WAVEFORM_RUN_ROLL <tst-pqm-waveform-run-roll>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Make sure that the settings menu is open.
    2. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    3. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.    
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click on the **Browse** button.
        - **Expected result:** An explorer window is displayed. Only directories 
          can be selected.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Select a directory and click on the **Open** button.
        - **Expected result:** The explorer window is closed and the path to that 
          directory is introduced in the browser section.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the **Run** button.
        - **Expected result:** The **LOG** section remains open, but its elements 
          can no longer be accessed and a .csv file is created in the selected 
          directory.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Click on the **Stop** button.
        - **Expected result:** The **LOG** section can now be accessed, and the 
          .csv file is populated with data.
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


Test 12: Data logging wrong path.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.WAVEFORM_LOG_WRONG

**RBP:** P3

**Description:** This test verifies if the logging mechanism works correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-waveform>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Make sure that the settings menu is open.
    2. Make sure that data acquisition is stopped.
        - If data acquisition is running, the log section cannot be accessed.
    3. Click on the **LOG** switch button.  
        - **Expected result:** A browse section is displayed.    
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Enter an invalid path in the line edit.
    5. Click on the **Run** button.
        - **Expected result:** The **LOG** section is closed and no .csv file is 
          created.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Click on the **Stop** button.
        - **Expected result:** The **LOG** section remains closed.
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


Settings tool
-------------

.. note::

    User guide: :ref:`Settings <pqm-settings>`.


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
^^^^^^^^^^^^^^^^^^

.. _pqm-emu-settings:

**PQM.Emu:**
    - Open Scopy.
    - Start the iio-emu process using "pqm" in the dropdown and "ip:127.0.0.1" 
      as URI.
    - Add the device in device browser.
    - Connect to "ip:127.0.0.1".

Test 1: Open Settings tool.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.SETTINGS_OPEN

**RBP:** P3

**Description:** This test verifies that the Settings tool opens correctly and 
is accessible without errors.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-settings>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Settings** tool.
    2. Observe the application's behavior.
        - **Expected result:** Three sections are displayed: **System Time**, 
          **Logging**, and **Config Values**.
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


Test 2: Read config values.
^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.SETTINGS_READ

**RBP:** P3

**Description:** This test verifies if the device attribute reading is done 
correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-settings>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Settings** tool.
    2. Scroll down in the **Config values** section until you find the **Read** 
       and **Set** buttons.
    3. Click on the **Read** button.
        - **Expected result:** All fields in the section are populated.  
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


Test 3: Write config values.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

**UID:** TST.PQM.SETTINGS_WRITE

**RBP:** P3

**Description:** This test verifies if the device attribute writing is done 
correctly.

**Test prerequisites:**
    - :ref:`TST.CONN.SUCC <tst-conn-succ>`.

**Preconditions:**
    - Scopy is installed on the system.
    - iio-emu is installed on the system.
    - Use :ref:`PQM.Emu <pqm-emu-settings>` setup.
    - OS: Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS.

**Steps:**
    1. Click on the **Settings** tool.
    2. Scroll down in the **Config values** section until you find the **Read** 
       and **Set** buttons.
    3. Click on the **Read** button.
        - **Expected result:** All fields in the section are populated.  
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Change the **MsV Threshold** value to 0.40.
    5. Click on the **Write** button.
    6. Click on the **Read** button.
        - **Expected result:** The **MsV Threshold** attribute value must be 0.40.
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
