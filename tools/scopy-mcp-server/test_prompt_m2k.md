# MCP Server Validation — ADALM2000 (M2K) at 192.168.2.1

## Prerequisites
- An ADALM2000 device connected at `192.168.2.1`
- Scopy is NOT running (this test starts it via MCP)

## Goal
Test as many MCP tools as possible against a real M2K device. This validates core lifecycle, tool switching, property read/write, run/stop, and per-instrument APIs. After each step, report the result (value returned or error). If a step fails, note it and continue.

---

## Phase 1: Core Lifecycle (start_scopy, add_device, connect_device)

1. **Start Scopy** — call `start_scopy`. Confirm it returns success.
2. **Add device** — call `add_device` with URI `192.168.2.1`. Record the returned device ID.
3. **Connect device** — call `connect_device` with the device ID (or index `0`). Confirm `true`.
4. **List tools** — call `scopy_call` with method `getTools`. List all available tools.
5. **List plugins** — call `scopy_call` with method `getPlugins` with arg1 `0`. List all plugins.
6. **Get devices name** — call `scopy_call` with method `getDevicesName`. List device names.

---

## Phase 2: Oscilloscope (osc, osc_channel, osc_channel_filter)

1. **Switch to Oscilloscope** — call `switch_tool` with `Oscilloscope`.
2. **Read time_base** — call `osc_read` with property `time_base`.
3. **Write time_base** — call `osc_write` with property `time_base`, value `0.001`.
4. **Read back time_base** — confirm it changed to `0.001`.
5. **Read running** — call `osc_read` with property `running`.
6. **Enable channel 0** — call `osc_channel_write` with property `enabled`, value `true`, index `0`.
7. **Read channel 0 volts_per_div** — call `osc_channel_read` with property `volts_per_div`, index `0`.
8. **Write channel 0 volts_per_div** — call `osc_channel_write` with property `volts_per_div`, value `0.5`, index `0`.
9. **Read back volts_per_div** — confirm it changed.
10. **Write channel 0 v_offset** — call `osc_channel_write` with property `v_offset`, value `0.1`, index `0`.
11. **Read channel filter enLow** — call `osc_channel_filter_read` with property `enLow`, index `0`.
12. **Start oscilloscope** — call `osc_write` with property `running`, value `true`.
13. **Wait 3 seconds** — let it acquire data.
14. **Read channel 0 measurements** — call `osc_channel_read` for properties: `mean`, `min`, `max`, `frequency`, `period`, `peak_to_peak`, `rms` (index `0`). Report all values.
15. **Enable cursors** — call `osc_write` with property `cursors`, value `true`.
16. **Set cursor positions** — call `osc_write` for `cursor_v1` = `-0.001`, `cursor_v2` = `0.001`, `cursor_h1` = `-0.5`, `cursor_h2` = `0.5`.
17. **Enable measure** — call `osc_write` with property `measure`, value `true`.
18. **Stop oscilloscope** — call `osc_write` with property `running`, value `false`.
19. **Read single** — call `osc_read` with property `single`.
20. **Set memory_depth** — call `osc_write` with property `memory_depth`, value `4096`.

---

## Phase 3: Signal Generator (siggen)

1. **Switch to Signal Generator** — call `switch_tool` with `Signal Generator`.
2. **Read channel 0 enabled** — call `siggen_read` with property `enabled`, channel `0`.
3. **Enable channel 0** — call `siggen_write` with property `enabled`, value `true`, channel `0`.
4. **Set waveform type** — call `siggen_write` with property `waveform_type`, value `0`, channel `0`. (0=sine)
5. **Set frequency** — call `siggen_write` with property `waveform_frequency`, value `1000`, channel `0`.
6. **Set amplitude** — call `siggen_write` with property `waveform_amplitude`, value `2`, channel `0`.
7. **Set offset** — call `siggen_write` with property `waveform_offset`, value `0`, channel `0`.
8. **Read back frequency** — call `siggen_read` with property `waveform_frequency`, channel `0`.
9. **Set channel 1** — repeat enable + set waveform_type `1` (square), frequency `5000`, amplitude `1` on channel `1`.
10. **Start signal gen** — call `siggen_write` with property `running`, value `true`.
11. **Wait 2 seconds**.
12. **Stop signal gen** — call `siggen_write` with property `running`, value `false`.

---

## Phase 4: Spectrum Analyzer (spectrum)

1. **Switch to Spectrum Analyzer** — call `switch_tool` with `Spectrum Analyzer`.
2. **Read startFreq** — call `spectrum_read` with property `startFreq`.
3. **Read stopFreq** — call `spectrum_read` with property `stopFreq`.
4. **Write startFreq** — call `spectrum_write` with property `startFreq`, value `100`.
5. **Write stopFreq** — call `spectrum_write` with property `stopFreq`, value `5000000`.
6. **Read topScale** — call `spectrum_read` with property `topScale`.
7. **Read bottomScale** — call `spectrum_read` with property `bottomScale`.
8. **Set units** — call `spectrum_write` with property `units`, value `dBFS`.
9. **Enable cursors** — call `spectrum_write` with property `cursors`, value `true`.
10. **Start spectrum** — call `spectrum_write` with property `running`, value `true`.
11. **Wait 3 seconds**.
12. **Stop spectrum** — call `spectrum_write` with property `running`, value `false`.

---

## Phase 5: Voltmeter (dmm)

1. **Switch to Voltmeter** — call `switch_tool` with `Voltmeter`.
2. **Read value_ch1** — call `dmm_read` with property `value_ch1`.
3. **Read value_ch2** — call `dmm_read` with property `value_ch2`.
4. **Set mode_ac_ch1** — call `dmm_write` with property `mode_ac_ch1`, value `true`.
5. **Read back mode_ac_ch1** — confirm it changed.
6. **Set mode_ac_ch1 back** — call `dmm_write` with property `mode_ac_ch1`, value `false`.
7. **Start DMM** — call `dmm_write` with property `running`, value `true`.
8. **Wait 2 seconds**.
9. **Read value_ch1 while running** — call `dmm_read` with property `value_ch1`. Report the voltage.
10. **Read value_ch2 while running** — call `dmm_read` with property `value_ch2`. Report the voltage.
11. **Enable histogram_ch1** — call `dmm_write` with property `histogram_ch1`, value `true`.
12. **Stop DMM** — call `dmm_write` with property `running`, value `false`.

---

## Phase 6: Power Supply (power)

1. **Switch to Power Supply** — call `switch_tool` with `Power Supply`.
2. **Read dac1_value** — call `power_read` with property `dac1_value`.
3. **Read dac2_value** — call `power_read` with property `dac2_value`.
4. **Set dac1_value** — call `power_write` with property `dac1_value`, value `3.3`.
5. **Enable dac1** — call `power_write` with property `dac1_enabled`, value `true`.
6. **Read back dac1_enabled** — confirm `true`.
7. **Set dac2_value** — call `power_write` with property `dac2_value`, value `-2.5`.
8. **Enable dac2** — call `power_write` with property `dac2_enabled`, value `true`.
9. **Read sync** — call `power_read` with property `sync`.
10. **Disable dac1** — call `power_write` with property `dac1_enabled`, value `false`.
11. **Disable dac2** — call `power_write` with property `dac2_enabled`, value `false`.

---

## Phase 7: Digital I/O (dio)

1. **Switch to Digital I/O** — call `switch_tool` with `Digital I/O`.
2. **Read dir channel 0** — call `dio_read` with property `dir`, channel `0`.
3. **Set dir channel 0 to output** — call `dio_write` with property `dir`, value `true`, channel `0`.
4. **Set out channel 0** — call `dio_write` with property `out`, value `true`, channel `0`.
5. **Read gpi channel 0** — call `dio_read` with property `gpi`, channel `0`.
6. **Start DIO** — call `dio_write` with property `running`, value `true`.
7. **Wait 1 second**.
8. **Stop DIO** — call `dio_write` with property `running`, value `false`.
9. **Reset dir channel 0** — call `dio_write` with property `dir`, value `false`, channel `0`.

---

## Phase 8: Logic Analyzer (logic)

1. **Switch to Logic Analyzer** — call `switch_tool` with `Logic Analyzer`.
2. **Read sampleRate** — call `logic_read` with property `sampleRate`.
3. **Write sampleRate** — call `logic_write` with property `sampleRate`, value `1000000`.
4. **Read bufferSize** — call `logic_read` with property `bufferSize`.
5. **Write bufferSize** — call `logic_write` with property `bufferSize`, value `4096`.
6. **Read enabledChannels** — call `logic_read` with property `enabledChannels`.
7. **Read channelNames** — call `logic_read` with property `channelNames`.

---

## Phase 9: Pattern Generator (pattern)

1. **Switch to Pattern Generator** — call `switch_tool` with `Pattern Generator`.
2. **Read enabledChannels** — call `pattern_read` with property `enabledChannels`.
3. **Read channelNames** — call `pattern_read` with property `channelNames`.

---

## Phase 10: Network Analyzer (network)

1. **Switch to Network Analyzer** — call `switch_tool` with `Network Analyzer`.
2. **Read min_freq** — call `network_read` with property `min_freq`.
3. **Read max_freq** — call `network_read` with property `max_freq`.
4. **Write min_freq** — call `network_write` with property `min_freq`, value `1000`.
5. **Write max_freq** — call `network_write` with property `max_freq`, value `1000000`.
6. **Read amplitude** — call `network_read` with property `amplitude`.
7. **Write amplitude** — call `network_write` with property `amplitude`, value `1`.
8. **Read samples_count** — call `network_read` with property `samples_count`.

---

## Phase 11: Calibration (calib)

1. **Switch to Calibration** — call `switch_tool` with `Calibration`.
2. **Read device temperature** — call `calib_call` with method `devTemp`, arg1 the device name from Phase 1.
3. **Reset calibration** — call `calib_call` with method `resetCalibration`.
4. **Calibrate ADC** — call `calib_call` with method `calibrateAdc`. Report `true`/`false`.
5. **Calibrate DAC** — call `calib_call` with method `calibrateDac`. Report `true`/`false`.

---

## ~~Phase 12-14: Generic Plugins — SKIPPED~~

> **Note:** Data Logger, IIO Explorer, and Register Map are generic plugins that are NOT loaded for M2K devices. These plugins are only available on generic IIO devices (e.g., Pluto). Test them using `test_prompt_pluto.md` instead.

---

## Phase 12: Cleanup

1. **Disconnect device** — call `disconnect_device`.
2. Confirm `true`.

---

## Summary

After completing all phases, compile a report:
- Total MCP tool calls made
- Number of successes vs failures
- List any tools/methods that returned errors or unexpected values
- List any tools/methods that could NOT be tested and why
