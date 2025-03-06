.. _dac_tests:

DAC Plugin - Test suite
=======================

.. note::

   User guide: :ref:`DAC Plugin user Guide<dac>`

The DAC plugin tests are a set of tests that are run to verify the DAC plugin in Scopy.

The following apply for all the test cases in this suite.
If the test case has special requirements, they will be listed in the test case section.


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
-------------------

.. _adalm_pluto_emu_setup:

**AdalmPluto.Emu:**
    - Open Scopy.
    - Start the IIO-EMU process.
    - Connect to the ADALM-Pluto device using the EMU dropdown.

.. _adalm_pluto_device_setup:

**AdalmPluto.Device:**
    - Open Scopy.
    - Connect the ADALM-Pluto device to the system.
    - Connect to the ADALM-Pluto device in Scopy using USB/network backend.

**Depends on::**
    - Test TST.PREFS.RESET

Prerequisites:
    - Scopy v2.0.0 or later with DAC plugin installed on the system.
    - Tests listed as dependencies are successfully completed.
    - Reset .ini files to default using the Preferences "Reset" button.

Test 1 - Compatibility with device
----------------------------------------------------

.. _TST.DAC.COMPAT:

**UID:** TST.DAC.COMPAT

**Description:** This test verifies that the DAC plugin is compatible with the selected device
and that the plugin is able to correctly parse it.

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ALL (Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS)

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer" and check the "Channels" list:
        - **Expected result:** The channels list contains the following: **cf-ad9361-dds-core-lpc:voltage0** 
          and **cf-ad9361-dds-core-lpc:voltage1**.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Switch the "Mode" to "DDS" and check the TX section:
        - **Expected result:** The TX section contains one entry with "TX1".
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


Test 2 - Buffer mode file loading
----------------------------------------------------

.. _TST.DAC.BUFFER_LOAD:

**UID:** TST.DAC.BUFFER_LOAD

**Description:** This test verifies that the DAC plugin is able to load a file in buffer mode.

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ALL (Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS)

**Resources:**
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv>`__
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sine_complex_1_30720_sr_15360000_samps_20000_ampl_32767.csv>`__
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sinewave_0.9_2ch.csv>`__
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/tutorial_chapters.json>`__

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. Select the *sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv* file using the 
       "Browse" button.
    4. Press the "Load" button.
        - **Expected result:**
            - The file should be loaded successfully.
            - The "File Size" field is populated with ** 40 ksamples**.
            - The Run button is enabled.
            - A new section called "DATA CONFIGURATION" should appear.
            - The bottom log section contains the following entry structure: *[timestamp] 
              Successfully loaded: [file name]*.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. While running, load a file with a lower number of samples using the browser and the file listed in the resources above: *sinewave_0.9_2ch.csv*.
        - **Expected result:** The "File Size" field is populated with ** 8444 samples**.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. While running, load a file with a lower number of samples using the browser and the file listed in the resources above: *sine_complex_1_30720_sr_15360000_samps_20000_ampl_32767.csv*.
        - **Expected result:** The "File Size" field is populated with ** 20 ksamples**.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Select the *tutorial_chapters.json* file from the resources provided above 
       using the "Browse" button.
    8. Press the "Load" button.
        - **Expected result:**
            - The file should NOT be loaded.
            - The Run button is disabled.
            - The bottom log section contains the following entry structure: *[timestamp] 
              Failed to load: [file name]*.
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


Test 3 - Buffer mode channel selection
----------------------------------------------------

.. _TST.DAC.BUFFER_CHANNEL_SELECT:

**UID:** TST.DAC.BUFFER_CHANNEL_SELECT

**Description:** This test verifies that the DAC plugin is able to validate channel selection in buffer mode.

**Preconditions:**
    - :ref:`AdalmPluto.Device<adalm_pluto_device_setup>`
    - OS: ANY

**Resources:**
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv>`__

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. Select the *sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv* file using the 
       "Browse" button.
    4. Press the "Load" button.
    5. Check the channels list for enabled channels:
        - **Expected result:** Both channels are enabled.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Press "Run".
        - **Expected result:** 
            - The "Run" button changes to "Stop".
            - The bottom log section contains the following entry structure: 
              *[timestamp]  Pushed 40000 samples, 160000 bytes (1/1 buffers).*
        - **Actual result:**

..
  Actual test result goes here.
..

    7. While the plugin is running, disable one of the channels.
        - **Expected result:**
            - The plugin keeps running.
            - The bottom log section contains the following entry structure: 
              *[timestamp]  Pushed 40000 samples, 80000 bytes (1/1 buffers).*
        - **Actual result:**

..
  Actual test result goes here.
..

    8. While the plugin is still running, disable the other channel.
         - **Expected result:** 
            - The plugin stops running and the button change back to "Run".
            - The bottom log section contains the following entry structure: 
              *[timestamp]  Unable to create buffer, no channel enabled.*
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


Test 4 - Buffer mode file size validation
---------------------------------------------------------

.. _TST.DAC.BUFFER_FILE_SIZE:

**UID:** TST.DAC.BUFFER_FILE_SIZE

**Description:** This test verifies that the DAC plugin is able to validate the file 
size in buffer mode.

**Preconditions:**
    - :ref:`AdalmPluto.Device<adalm_pluto_device_setup>`
    - OS: ANY

**Resources:**
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv>`__

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. Select the *sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv* file using the 
       "Browse" button.
    4. Press the "Load" button.
    5. Check the "File Size" field.
        - **Expected result:** The "File Size" field is populated with **40 ksamples**.
        - **Actual result:**

..
  Actual test result goes here.
..

    6. Change the "File Size" field to a value higher than the actual file size (write 50 and press Enter).
        - **Expected result:** The field value remains 40 ksamples since that's 
          the maximum number of samples in the file.
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Change the "File Size" field to a value lower than the actual file size (write 30 and press Enter).
        - **Expected result:** The field value changes to 30 ksamples.
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Change the "File Size" using the "+" button.
        - **Expected result:** The field value changes to the 31.
        - **Actual result:**

..
  Actual test result goes here.
..

    9. Change the "File Size" using the "-" button.
        - **Expected result:** The field value changes to the 30.
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


Test 5 - Cyclic buffer mode
---------------------------------------------

.. _TST.DAC.BUFFER_CYCLIC:

**UID:** TST.DAC.BUFFER_CYCLIC

**Description:** This test verifies the DAC plugin buffer generation in cyclic mode.

**Depends on::**
    - Test TST.ADC.*
    - Test TST.Debugger.*

**Preconditions:**
    - :ref:`AdalmPluto.Device<adalm_pluto_device_setup>`
    - ADALM-Pluto with loopback cable between RX and TX.
    - OS: ALL (Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS)

**Resources:**
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv>`__

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. Select the *sine_complex_1_15360_sr_15360000_samps_40000_ampl_32767.csv* file using the 
       "Browse" button.
    4. Press the "Load" button.
    5. Press "Run".
    6. Open the Debugger plugin.
    7. In the **ad9361-phy** device, set the **frequency** attribute for the **altvoltage1 TX_LO** 
       and **altvoltage0 RX_LO** channels to 2450000000.
    8. Open the ADC plugin.
    9. Change the Buffer Size to 4096.
    10. Press "Run".
    11. Check the ADC plugin for the generated signal.
         - **Expected result:** A sinewave is seen on the ADC plot.
         - **Actual result:**

..
  Actual test result goes here.
..

    12. Stop the DAC plugin while monitoring the ADC plot.
         - **Expected result:** The sinewave no longer appears.
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


Test 6 -  DDS modes
----------------------------------

.. _TST.DAC.DDS_MODES:

**UID:** TST.DAC.DDS_MODES

**Description:** This test verifies the DAC plugin DDS modes.

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ANY

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "DDS".
    3. Change the "DDS Mode" to **One CW Tone**.
        - **Expected result:**
            - One configuration panel appears.
            - The configuration panel contains the following fields:
                - **frequency**: 9.279985 MHz
                - **scale**: -Inf db
                - **phase**: 90
                - the channel name: "Tone 1 - TX1_I_F1"
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Change the "DDS Mode" to **Two CW Tones**.
        - **Expected result:**
            - Two configuration panels appear having the same fields as in the previous step.
            - The second channel name: "Tone 2 - TX1_I_F2"
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Change the "DDS Mode" to **Independent I/Q Control**.
        - **Expected result:**
            - Four configuration panels appear having the same fields as in the previous step.
            - The third channel name: "Tone 1 - TX1_Q_F1"
            - The fourth channel name: "Tone 2 - TX1_Q_F2"
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


Test 7 - DDS mode sinewave generation
------------------------------------------------------

.. _TST.DAC.DDS_SINEWAVE:

**UID:**
    TST.DAC.DDS_SINEWAVE

**Description:**
    This test verifies the DAC plugin DDS mode sinewave generation.

**Depends on::**
    - Test TST.DEBUGGER.*
    - Test TST.ADC.*

**Preconditions:**
    - :ref:`AdalmPluto.Device<adalm_pluto_device_setup>`
    - ADALM-Pluto with loopback cable between RX and TX.
    - OS: ALL (Windows, Linux-x86_64, Linux-arm64, Linux-arm32, macOS)

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "DDS".
    3. Change the "DDS Mode" to **One CW Tone**.
    4. Change the "frequency" to 1 MHz.
    5. Change the "scale" to 0 db.
    6. Open the Debugger plugin.
    7. In the **ad9361-phy** device, set the **frequency** attribute for the **altvoltage1 TX_LO** and
       **altvoltage0 RX_LO** channels to 2450000000.
    8. Open the ADC plugin.
    9. Enable all the Measurements.
    10. Press "Run".
    11. Check the plot for the generated sinewave.
         - **Expected result:**
            - A sinewave should be seen on the ADC plot.
            - The measurements show a frequency of 1 MHz.
         - **Actual result:**

..
  Actual test result goes here.
..

    12. In the DAC plugin change the "DDS Mode" to **Disabled**.
         - **Expected result:** The sinewave no longer appears on the ADC plot.
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


Test 8 - DDS mode attributes refresh
-----------------------------------------------------

.. _TST.DAC.DDS_ATTRS:

**UID:** TST.DAC.DDS_ATTRS

**Description:** This test verifies the DAC plugin DDS mode attributes refresh.

**Depends on::**
    - Test TST.DEBUGGER.*

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ANY

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "DDS".
    3. Change the "DDS Mode" to **One CW Tone**.
    4. Change the "frequency" to 1 MHz.
    5. Open the Debugger plugin.
    6. In the **cf-ad9361-dds-core-lpc** device, set the **frequency** attribute 
       for the **altvoltage0 TX1_I_F1** to 2000000.
    7. Go to the DAC Plugin and click the refresh attributes button.
        - **Expected result:** The frequency field is updated to 2 MHz.
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



Test 9 - Plugin tutorial
---------------------------------

.. _TST.DAC.TUTORIAL:

**UID:** TST.DAC.TUTORIAL

**Description:** This test verifies the DAC plugin tutorial.

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ANY

**Steps:**
    1. Open the DAC plugin.
    2. Click the "Info" button.
        - **Expected result:** A pop-up window with 2 buttons (Tutorial and Documentation)
          should appear.
        - **Actual result:**

..
  Actual test result goes here.
..

    3. Click the "Documentation" button.
        - **Expected result:** The documentation page opens in the default browser.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Click the "Tutorial" button.
        - **Expected result:**
            - The tutorial opens as a walkthrough in the plugin.
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


Test 10 - Channel attributes
--------------------------------------------

.. _TST.DAC.CHANNEL_ATTRS:

**UID:** TST.DAC.CHANNEL_ATTRS

**Description:** This test verifies the DAC plugin channel attributes.

**Preconditions:**
    - :ref:`AdalmPluto.Emu<adalm_pluto_emu_setup>`
    - OS: ANY

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. In the "Channels" panel click on the first channel "cf-ad9361-dds-core-lpc:voltage0".
        - **Expected result:** The channel attributes are displayed in the right side menu.
        - **Actual result:**

..
  Actual test result goes here.
..

    4. Change the **sampling_frequency** attribute to 38400000.
        - **Expected result:** The attribute is updated successfully.
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


Test 11 - Repeat data option
--------------------------------------------

.. _TST.DAC.REPEAT_DATA:

**UID:** TST.DAC.REPEAT_DATA

**Description:** This test verifies the DAC plugin repeat data option.
The data sets loaded from the file should be repeated to fill in all the enabled
channels.

**Preconditions:**
    - :ref:`AdalmPluto.Device<adalm_pluto_device_setup>`
    - Enable 2 TX channels on your Pluto device.
    - OS: ANY

**Resources:**
    - `<https://github.com/analogdevicesinc/scopy/blob/main/plugins/dac/res/csv/sinewave_0.9_2ch.csv>`__

**Steps:**
    1. Open the DAC plugin.
    2. Switch the "Mode" to "Buffer".
    3. Browse and select the *sinewave_0.9_2ch.csv* file.
    4. Press the "Load" button.
        - **Expected result:** The first 2 channels are enabled.
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Enable the remaining 2 channels from the channel list.
    6. Set the **Repeat data** option to true and press **Run**.
        - **Expected result:** The console log displays the following message: *Pushed 8444 samples, 67552 bytes (1/1 buffers).*
        - **Actual result:**

..
  Actual test result goes here.
..

    7. Set the **Repeat data** option to false and press **Run**.
        - **Expected result:** The console log displays the following message: *Not enough data columns for all enabled channels.*
        - **Actual result:**

..
  Actual test result goes here.
..

    8. Leave the **Repeat data** as is, disable the last 2 channels and press **Run**:
        - **Expected result:** The console log displays the following message: *Pushed 8444 samples, 33776 bytes (1/1 buffers).*
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

