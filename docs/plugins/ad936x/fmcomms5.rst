.. _fmcomms5:

FMCOMMS5
================================================================================

Description
---------------------------------

The FMCOMMS5 plugin for Scopy provides an interface to control and monitor the 
AD9361-based FMCOMMS5 hardware. This plugin is designed to work seamlessly with 
Scopy, allowing users to configure, calibrate, and operate the FMCOMMS5 board for 
multi-channel RF applications. The plugin supports real-time updates: changing any 
field will immediately write changes to the hardware and read them back to ensure 
validity. If a setting is not supported by the hardware or FPGA, the GUI will 
revert to a valid value.

If you manipulate the device outside of Scopy (e.g., via ``/sys/bus/iio/devices/...``), 
you can refresh the plugin view to synchronize settings. For advanced or unsupported 
features, refer to the FMCOMMS5 Advanced Plugin.

The FMCOMMS5 view is divided into several sections:
  - **Device Global Settings**
  - **Synchronization Controls**
  - **Receive Chain**
  - **Transmit Chain**

.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/refs/heads/doc_resources/resources/fmcomms5/fmcomms5.png
    :align: center


Device Global Settings
---------------------------------

  - **Active ENSM:** Displays the active mode of the Enable State Machine.
    (`Enable State Machine Controls <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#enable_state_machine_controls>`_)
  - **ENSM Modes:** Selects one of the available modes: FDD and TDD.
    (`Enable State Machine Controls <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#enable_state_machine_controls>`_)
  - **Calibration Mode:** Displays the active calibration mode.
    (`Calibration Mode Controls <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#calibration_mode_controls>`_)
  - **TRX Rate Governor:** Displays the active option of the Rate Governors.
    (`Rate Governors <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rate_governors>`_)
  - **Filter FIR configuration:** Allows a FIR filter configuration to be
    loaded from a file.
    (`Digital FIR Filter Controls <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#digital_fir_filter_controls>`_)
  - **RX Path Rates:** Lists the rates of: BBPLL, ADC, R2, R1, RF, RXSAMP.
    (`List Chosen RX Path Rates <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#list_chosen_rx_path_rates>`_)
  - **TX Path Rates:** Lists the rates of: TXSAMP, TF, T1, T2, DAC, BBPLL.
    (`List Chosen TX Path Rates <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#list_chosen_tx_path_rates>`_)
  - **XO Coarse Tune:** Selects the attribute for a coarse tune.
    (`DCXO Tuning <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#dcxo_tuning_digital_controlled_crystal_oscillator>`_)


Synchronization Controls
---------------------------------

  - **MCS (Multi-Chip Synchronization):** Controls for synchronizing multiple AD9361 chips on the FMCOMMS5 board.
  - **SYNC Status:** Displays the synchronization status of all channels.
  - **Trigger Synchronization:** Initiate a hardware or software sync event.


Receive Chain
---------------------------------

  - **RF Bandwidth(MHz):** Configures RX analog filters: RX TIA LPF and RX BB
    LPF. (`RX RF Bandwidth Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rx_rf_bandwidth_control>`_)
  - **Sampling Rate(MSPS):** Selects the sample rate of the ADC.
    (`Setting/Querying the RX Sample Rate <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#setting_/_querying_the_rx_sample_rate>`_)
  - **RF Port Select:** Selects the RF port to be used. Can be either any of
    the inputs on the Rx input mux (in single ended or differential) or the
    Tx monitor input.
    (`RF Port Selection <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rf_port_selection>`_)
  - **RX LO Frequency(MHz):** Selects the RX local oscillator frequency.
    Range 70MHz to 6GHz with 1Hz tuning granularity.
    (`Local Oscillator Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#local_oscillator_control(lo)>`_)
  - **External RX LO:** Allows switching between external and internal LO on
    the fly.
    (`External LO Support <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#external_lo_support>`_)
  - **Fastlock Profile:** Selects one of the 8 available profiles of
    frequency configuration information.
    (`Fastlock Mode <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#fastlock_mode>`_)
   
    - **Store:** Stores the current frequency configuration into the profile
      pointed by **Fastlock Profile**.
    - **Recall:** Recalls the profile pointed by **Fastlock Profile**.
  
  - **Tracking** (`Calibration Tracking Controls <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#calibration_tracking_controls>`_)
  
    - **Quadrature**
    - **RF DC**
    - **BB DC**
 
  - **RX**

    - **Hardware Gain(dB):** Controls the RX gain only in Manual Gain
      Control Mode (MGC).
      (`MGC Setting the Current Gain <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#MGC_setting_the_current_gain>`_)
    - **RSSI(dB):** Displays the received strength signal level.
      (`Received Strength Signal Indicator (RSSI) <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#received_strength_signal_indicator_(rssi)>`_)
    - **Gain Control:** Displays the active gain mode.
      (`RX Gain Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rx_gain_control>`_)
    
Transmit Chain
---------------------------------

  - **RF Bandwidth(MHz):** Configures TX analog filters: TX BB LPF and  TX
    Secondary LPF.
    (`TX RF Bandwidth Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#tx_rf_bandwidth_control>`_)
  - **Sampling Rate(MSPS):** Selects the sample rate of the DAC.
    (`Setting/Querying the TX Sample Rate <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#setting_/_querying_the_tx_sample_rate>`_)
  - **RF Port Select:** Selects the RF port to be used.
    (`RF Port Selection <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#rf_port_selection>`_)
  - **TX LO Frequency(MHz):** Selects the TX local oscillator frequency.
    Range 70MHz to 6GHz with 1Hz tuning granularity.
    (`Local Oscillator Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#local_oscillator_control(lo)>`_)
  - **External TX LO:** Allows switching between external and internal LO on
    the fly.
    (`External LO Support <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#external_lo_support>`_)
  - **Fastlock Profile:** Selects one of the 8 available profiles of
    frequency configuration information.
    (`Fastlock Mode <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#fastlock_mode>`_)
  
    - **Store:** Stores the current frequency configuration into the profile
      pointed by **Fastlock Profile**.
    - **Recall:** Recalls the profile pointed by **Fastlock Profile**.
  
  - **TX**

      - **Attenuation(dB):** Individually controls attenuation for TX1 and
        TX2. The range is from 0 to -89.75 dB in 0.25dB steps.
        (`TX Attenuation Control <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#tx_attenuation_control>`_)
      - **RSSI(dB):** TX Received Strength Signal Indicator. Active when
        TX_MONITOR port is selected in the RX **RF Port Select**.
        (`TX Received Strength Signal Indicator (TX RSSI) <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361#tx_received_strength_signal_indicator_tx_rssi>`_)



.. image:: https://raw.githubusercontent.com/analogdevicesinc/scopy/refs/heads/doc_resources/resources/fmcomms5/fmcomms5.png
    :align: center

Block Diagram
---------------------------------

  The block diagram of the FMCOMMS5 device is available in the "Block Diagram" tab for your reference.

References
---------------------------------

- `FMCOMMS5 Plugin Wiki <https://wiki.analog.com/resources/tools-software/linux-software/fmcomms5_plugin>`_
- `AD9361 Linux Driver <https://wiki.analog.com/resources/tools-software/linux-drivers/iio-transceiver/ad9361>`_
- `Scopy User Guide <https://wiki.analog.com/resources/tools-software/scopy>`_
