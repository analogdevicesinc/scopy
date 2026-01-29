.. _datalogger_tests:


Datalogger Test Suite
================================================================================

.. note::

  User guide :ref:`Data Logger user guide <datalogger>`

The following apply to all tests below, any special requirements will be mentioned.

**Prerequisites:**
    - Scopy v2.0.0 or later with Data Logger installed on the system
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

.. _datalogger_adalm_pluto_emu_setup:

**AdalmPluto.Emu:**
    - Open Scopy
    - Start the iio-emu process using Pluto option from the dropdown
    - Connect to the Pluto Device

.. _datalogger_adalm_pluto_device_setup:

**AdalmPluto.Device:**
    - Open Scopy
    - Connect to the Pluto Device using IP or USB

.. _datalogger_m2k_emu_setup:

**M2k.Emu:**
    - Open Scopy
    - Start the iio-emu process using ADALM2000 option from the dropdown
    - Connect to the M2k Device

.. _TST_DATALOGGER_ENABLE_DISABLE_DATA_LOGGER_CHANNEL:

Test 1: Enable and Disable Data Logger Channel
------------------------------------------------

**UID:** TST.DATALOGGER.ENABLE_DISABLE_DATA_LOGGER_CHANNEL

**RBP:** P0

**Description:** Enable the channel and check if the channel is enabled

**OS:** ANY

**Preconditions:**
    - Connect to :ref:`AdalmPluto.Emu <datalogger_adalm_pluto_emu_setup>` or 
      :ref:`AdalmPluto.Device <datalogger_adalm_pluto_device_setup>`

**Steps:**
    1. Open Data Logger plugin
    2. Using channel menu enable channel **voltage0** of **xadc** device
    3. Press **Run** button
    4. Wait for some data to be collected
        - **Expected result:** Curve will be displayed on the plot
        - **Actual result:**

..
  Actual test result goes here.
..

    5. Using channel menu disable channel **voltage0** of **xadc** device
        - **Expected result:** Curve will be removed from the plot
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


.. _TST_DATALOGGER_RUN_STOP_DATA_LOGGING:

