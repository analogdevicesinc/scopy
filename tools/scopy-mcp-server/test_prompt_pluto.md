# MCP Server Validation — Pluto (AD936X) Emulator at 127.0.0.1

## Prerequisites
- `iio-emu pluto` running at `127.0.0.1`
- Scopy IS already running (this test attaches to it — do NOT call `start_scopy`)

## Goal
Test all AD936X plugin APIs, AD936X Advanced APIs, and all generic plugin tools (Data Logger, IIO Explorer, Register Map) against an emulated Pluto device. This validates the MCP attach flow, method-based call tools, and the full AD936X API surface. After each step, report the result. If a step fails, note it and continue.

---

## Phase 1: Attach & Connect (no start_scopy)

1. **Add device** — call `add_device` with URI `127.0.0.1`. Record the returned device ID.
2. **Connect device** — call `connect_device` with the device ID (or index `0`). Confirm `true`.
3. **List tools** — call `scopy_call` with method `getTools`. List all available tools.
4. **List plugins** — call `scopy_call` with method `getPlugins` with arg1 `0`.
5. **Get tool button state** — call `scopy_call` with method `getToolBtnState`, arg1 `AD936X`.
6. **Get devices name** — call `scopy_call` with method `getDevicesName`.

---

## Phase 2: AD936X Plugin — Read All Parameters

1. **Switch to AD936X** — call `switch_tool` with `AD936X`.
2. **Get available tools** — call `ad936x_call` with method `getTools`.
3. **Check if FMCOMMS5** — call `ad936x_call` with method `isFmcomms5`.

### RX Parameters
4. **Get RX LO Frequency** — call `ad936x_call` with method `getRxLoFrequency`.
5. **Get RX Sampling Frequency** — call `ad936x_call` with method `getRxSamplingFrequency`.
6. **Get RX RF Bandwidth** — call `ad936x_call` with method `getRxRfBandwidth`.
7. **Get RX RF Port Select** — call `ad936x_call` with method `getRxRfPortSelect`.
8. **Get RX Path Rates** — call `ad936x_call` with method `getRxPathRates`.
9. **Get RX Gain Control Mode ch0** — call `ad936x_call` with method `getRxGainControlMode`, arg1 `0`.
10. **Get RX Hardware Gain ch0** — call `ad936x_call` with method `getRxHardwareGain`, arg1 `0`.
11. **Get RX RSSI ch0** — call `ad936x_call` with method `getRxRssi`, arg1 `0`.

### TX Parameters
12. **Get TX LO Frequency** — call `ad936x_call` with method `getTxLoFrequency`.
13. **Get TX Sampling Frequency** — call `ad936x_call` with method `getTxSamplingFrequency`.
14. **Get TX RF Bandwidth** — call `ad936x_call` with method `getTxRfBandwidth`.
15. **Get TX RF Port Select** — call `ad936x_call` with method `getTxRfPortSelect`.
16. **Get TX Path Rates** — call `ad936x_call` with method `getTxPathRates`.
17. **Get TX Hardware Gain ch0** — call `ad936x_call` with method `getTxHardwareGain`, arg1 `0`.
18. **Get TX RSSI ch0** — call `ad936x_call` with method `getTxRssi`, arg1 `0`.

### Misc Parameters
19. **Get ENSM Mode** — call `ad936x_call` with method `getEnsmMode`.
20. **Get Calib Mode** — call `ad936x_call` with method `getCalibMode`.
21. **Get XO Correction** — call `ad936x_call` with method `getXoCorrection`.
22. **Get TRX Rate Governor** — call `ad936x_call` with method `getTrxRateGovernor`.
23. **Is BB DC Offset Tracking** — call `ad936x_call` with method `isBbDcOffsetTrackingEnabled`.
24. **Is RF DC Offset Tracking** — call `ad936x_call` with method `isRfDcOffsetTrackingEnabled`.
25. **Is Quadrature Tracking** — call `ad936x_call` with method `isQuadratureTrackingEnabled`.

---

## Phase 3: AD936X Plugin — Write & Verify Parameters

1. **Set RX LO Frequency** — call `ad936x_call` with method `setRxLoFrequency`, arg1 `2400000000`.
2. **Read back RX LO** — call `ad936x_call` with method `getRxLoFrequency`. Confirm it changed.
3. **Set TX LO Frequency** — call `ad936x_call` with method `setTxLoFrequency`, arg1 `2400000000`.
4. **Read back TX LO** — confirm it changed.
5. **Set RX Sampling Frequency** — call `ad936x_call` with method `setRxSamplingFrequency`, arg1 `30720000`.
6. **Read back RX Sampling** — confirm it changed.
7. **Set TX Sampling Frequency** — call `ad936x_call` with method `setTxSamplingFrequency`, arg1 `30720000`.
8. **Set RX RF Bandwidth** — call `ad936x_call` with method `setRxRfBandwidth`, arg1 `18000000`.
9. **Read back RX BW** — confirm it changed.
10. **Set TX RF Bandwidth** — call `ad936x_call` with method `setTxRfBandwidth`, arg1 `18000000`.
11. **Set RX Gain Control Mode** — call `ad936x_call` with method `setRxGainControlMode`, arg1 `0`, arg2 `manual`.
12. **Read back RX Gain Mode** — confirm `manual`.
13. **Set RX Hardware Gain** — call `ad936x_call` with method `setRxHardwareGain`, arg1 `0`, arg2 `30`.
14. **Read back RX HW Gain** — confirm it changed.
15. **Set TX Hardware Gain** — call `ad936x_call` with method `setTxHardwareGain`, arg1 `0`, arg2 `-10`.
16. **Read back TX HW Gain** — confirm it changed.
17. **Toggle BB DC Offset Tracking** — call `ad936x_call` with method `setBbDcOffsetTrackingEnabled`, arg1 `false`, then read back, then set `true`.
18. **Toggle RF DC Offset Tracking** — same pattern with `setRfDcOffsetTrackingEnabled`.
19. **Toggle Quadrature Tracking** — same pattern with `setQuadratureTrackingEnabled`.
20. **Refresh** — call `ad936x_call` with method `refresh`.

---

## Phase 4: AD936X Plugin — Widget API

1. **Get widget keys** — call `ad936x_call` with method `getWidgetKeys`. List all available keys.
2. **Read first 3 widgets** — for each of the first 3 keys returned, call `ad936x_call` with method `readWidget`, arg1 = key.
3. **Write a widget** — pick a writable widget key, call `ad936x_call` with method `writeWidget`, arg1 = key, arg2 = a valid value.
4. **Read it back** — confirm the widget value changed.

---

## Phase 5: AD936X Advanced Plugin

> **Important:** You MUST call `switch_tool` with `AD936X Advanced` before reading any advanced parameters. The `ad936x_advanced` JS object is only active when the "AD936X Advanced" tool is the current tool. Using `switchSubtab` alone is NOT sufficient.

1. **Switch to AD936X Advanced tool** — call `switch_tool` with `AD936X Advanced`. Confirm `true`.
2. **Get tools** — call `ad936x_advanced_call` with method `getTools`.

### Read a sampling of advanced parameters (pick ~10 representative ones):
3. **AGC Gain Update Interval** — call `ad936x_advanced_call` with method `getAgcGainUpdateInterval`.
4. **AGC Inner Thresh High** — call `ad936x_advanced_call` with method `getAgcInnerThreshHigh`.
5. **AGC Outer Thresh High** — call `ad936x_advanced_call` with method `getAgcOuterThreshHigh`.
6. **RSSI Delay** — call `ad936x_advanced_call` with method `getRssiDelay`.
7. **RSSI Duration** — call `ad936x_advanced_call` with method `getRssiDuration`.
8. **Temp Sense Measurement Interval** — call `ad936x_advanced_call` with method `getTempSenseMeasurementInterval`.
9. **Is FDD Mode Enabled** — call `ad936x_advanced_call` with method `isFddModeEnabled`.
10. **Loopback** — call `ad936x_advanced_call` with method `getLoopback`.
11. **BIST PRBS** — call `ad936x_advanced_call` with method `getBistPrbs`.
12. **Clk Output Mode Select** — call `ad936x_advanced_call` with method `getClkOutputModeSelect`.

### Write & verify a few advanced parameters:
13. **Set AGC Gain Update Interval** — call `ad936x_advanced_call` with method `setAgcGainUpdateInterval`, arg1 = a valid value. Read back to confirm.
14. **Set Temp Sense Measurement Interval** — call `ad936x_advanced_call` with method `setTempSenseMeasurementInterval`, arg1 = a valid value. Read back.
15. **Get widget keys** — call `ad936x_advanced_call` with method `getWidgetKeys`.
16. **Read a widget** — call `ad936x_advanced_call` with method `readWidget`, arg1 = first key.
17. **Refresh** — call `ad936x_advanced_call` with method `refresh`.

---

## Phase 6: Data Logger (Generic Plugin)

1. **Switch to Data Logger** — call `switch_tool` with `Data Logger`.
2. **Show available monitors** — call `datalogger_call` with method `showAvailableMonitors`. List them all.
3. **Show available devices** — call `datalogger_call` with method `showAvailableDevices`.
4. **Show monitors of device** — call `datalogger_call` with method `showMonitorsOfDevice`, arg1 `xadc`.
5. **Get tool list** — call `datalogger_call` with method `getToolList`. Note the exact tool name.

### Test enableMonitor (plugin-level — recently fixed):
6. **Enable xadc:temp0** — call `datalogger_call` with method `enableMonitor`, arg1 `xadc:temp0`. Confirm `OK`.
7. **Start data logger** — call `datalogger_call` with method `setRunning`, arg1 `true`.
8. **Wait 5 seconds** to collect data.
9. **Check running** — call `datalogger_call` with method `getRunning`. Confirm `true`.
10. **Stop data logger** — call `datalogger_call` with method `setRunning`, arg1 `false`.
11. **Disable xadc:temp0** — call `datalogger_call` with method `disableMonitor`, arg1 `xadc:temp0`. Confirm `OK`.

### Test enableMonitorOfTool (tool-level):
12. **Enable xadc:voltage0** — call `datalogger_call` with method `enableMonitorOfTool`, arg1 = exact tool name from step 5, arg2 `xadc:voltage0`.
13. **Start/run 3 seconds/stop** — same as above.
14. **Disable** — call `datalogger_call` with method `disableMonitorOfTool`, arg1 = tool name, arg2 `xadc:voltage0`.

### Test tool management:
15. **Create new tool** — call `datalogger_call` with method `createTool`. Record the returned string.
16. **Get tool list** — call `datalogger_call` with method `getToolList`. The new tool may appear with an auto-generated name (e.g., `Data Logger 1`). If the new tool doesn't appear, note this as a known issue and continue with the default tool.
17. **Enable monitor on new tool** — if the new tool appeared, call `datalogger_call` with method `enableMonitorOfTool`, arg1 = new tool name, arg2 `xadc:temp0`. Otherwise skip.

### Test display settings:
18. **Set min/max** — call `datalogger_call` with method `setMinMax`, arg1 `true`.
19. **Change precision** — call `datalogger_call` with method `changePrecision`, arg1 `3`.
20. **Set Y-axis min** — call `datalogger_call` with method `setMinYAxis`, arg1 `0`.
21. **Set Y-axis max** — call `datalogger_call` with method `setMaxYAxis`, arg1 `100`.
22. **Set display mode** — call `datalogger_call` with method `setDisplayMode`, arg1 = tool name, arg2 `1`.
23. **Clear data** — call `datalogger_call` with method `clearData`.

### Test data logging to file:
24. **Set log path** — call `datalogger_call` with method `setLogPathOfTool`, arg1 = tool name, arg2 `/tmp/scopy_mcp_test.csv`.
25. **Enable a monitor, start, wait 3s, stop**.
26. **Log at path** — call `datalogger_call` with method `logAtPathForTool`, arg1 = tool name, arg2 `/tmp/scopy_mcp_test.csv`.
27. **Test continuous logging** — call `datalogger_call` with method `continuousLogAtPathForTool`, arg1 = tool name, arg2 `/tmp/scopy_mcp_continuous.csv`. Start, wait 3s, then call `stopContinuousLogForTool`.

### Test monitor customization:
28. **Set monitor display name** — call `datalogger_call` with method `setMonitorDisplayName`, arg1 = tool name, arg2 `xadc:temp0`, arg3 `Temperature`.
29. **Set unit name** — call `datalogger_call` with method `setMonitorUnitOfMeasurementName`, arg1 = tool name, arg2 `xadc:temp0`, arg3 `Celsius`.
30. **Set unit symbol** — call `datalogger_call` with method `setMonitorUnitOfMeasurementSymbol`, arg1 = tool name, arg2 `xadc:temp0`, arg3 `°C`.

### Test tool rename:
31. **Set tool name** — call `datalogger_call` with method `setToolName`, arg1 = current tool name, arg2 `My Custom Logger`.
32. **Get tool list** — confirm rename.

---

## Phase 7: IIO Explorer (Generic Plugin)

1. **Switch to IIO Explorer** — call `switch_tool` with `Debugger`.
2. **Get visible items** — call `iioExplorer_call` with method `getVisibleItemCount`.
3. **Get root children** — call `iioExplorer_call` with method `getChildNames`, arg1 `context0`. (Note: empty string does NOT work — `context0` is the root node.)
4. **Get child count** — call `iioExplorer_call` with method `getChildCount`, arg1 `context0`.

### Tree navigation:
5. **Select ad9361-phy** — call `iioExplorer_call` with method `selectItemByPath`, arg1 `context0/ad9361-phy`.
6. **Get selected item name** — call `iioExplorer_call` with method `getSelectedItemName`.
7. **Get selected item type** — call `iioExplorer_call` with method `getSelectedItemType`.
8. **Get selected item path** — call `iioExplorer_call` with method `getSelectedItemPath`.
9. **Expand item** — call `iioExplorer_call` with method `expandItem`, arg1 `context0/ad9361-phy`.
10. **Check expanded** — call `iioExplorer_call` with method `isItemExpanded`, arg1 `context0/ad9361-phy`. Confirm `true`.
11. **Collapse item** — call `iioExplorer_call` with method `collapseItem`, arg1 `context0/ad9361-phy`.

### Attribute read/write:
12. **Read an attribute** — call `iioExplorer_call` with method `readAttributeValue`, arg1 = a full attribute path (e.g., `context0/ad9361-phy/voltage0/hardwaregain`).
13. **Trigger read** — call `iioExplorer_call` with method `triggerRead`.
14. **Write an attribute** — call `iioExplorer_call` with method `writeAttributeValue`, arg1 = attribute path, arg2 = a valid value.
15. **Read back** — confirm the write took effect.

### Watchlist:
16. **Select an item** — select a channel attribute.
17. **Add to watchlist** — call `iioExplorer_call` with method `addCurrentItemToWatchlist`.
18. **Add by path** — call `iioExplorer_call` with method `addItemToWatchlistByPath`, arg1 = another attribute path.
19. **Get watchlist** — call `iioExplorer_call` with method `getWatchlistPaths`. Confirm both items present.
20. **Refresh watchlist** — call `iioExplorer_call` with method `refreshWatchlist`.
21. **Remove from watchlist** — call `iioExplorer_call` with method `removeItemFromWatchlistByPath`, arg1 = first path.
22. **Clear watchlist** — call `iioExplorer_call` with method `clearWatchlist`.
23. **Get watchlist** — confirm empty.

### Tabs & Search:
24. **Set search text** — call `iioExplorer_call` with method `setSearchText`, arg1 `voltage`.
25. **Get search text** — confirm `voltage`.
26. **Get visible items after search** — call `iioExplorer_call` with method `getVisibleItemCount`. Should be filtered.
27. **Clear search** — call `iioExplorer_call` with method `setSearchText`, arg1 (empty).
28. **Switch to log tab** — call `iioExplorer_call` with method `showLogTab`.
29. **Get log content** — call `iioExplorer_call` with method `getLogContent`.
30. **Clear log** — call `iioExplorer_call` with method `clearLog`.
31. **Switch to code generator** — call `iioExplorer_call` with method `showCodeGeneratorTab`.
32. **Get generated code** — call `iioExplorer_call` with method `getGeneratedCode`.
33. **Switch to attributes tab** — call `iioExplorer_call` with method `showIIOAttributesTab`.
34. **Set details view tab** — call `iioExplorer_call` with method `setDetailsViewTab`, arg1 `0`, then `1`.

---

## Phase 8: Register Map (Generic Plugin)

1. **Switch to Register Map** — call `switch_tool` with `Register Map`.
2. **Get available devices** — call `regmap_call` with method `getAvailableDevicesName`.
3. **Set device** — call `regmap_call` with method `setDevice`, arg1 = `ad9361-phy` (or first available).
4. **Get current device** — call `regmap_call` with method `getDevice`. Confirm it matches.
5. **Read register 0x000** — call `regmap_call` with method `readRegister`, arg1 `0x000`.
6. **Get value of register** — call `regmap_call` with method `getValueOfRegister`, arg1 `0x000`.
7. **Get register info** — call `regmap_call` with method `getRegisterInfo`, arg1 `0x000`.
8. **Get register bitfields** — call `regmap_call` with method `getRegisterBitFieldsInfo`, arg1 `0x000`.
9. **Search registers** — call `regmap_call` with method `search`, arg1 `gain`.
10. **Read a range** — call `regmap_call` with method `readInterval`, arg1 `0x000`, arg2 `0x00F`. Note: this method returns `void` (updates UI only), so the result will be `undefined` — that is expected.
11. **Check autoread** — call `regmap_call` with method `isAutoreadEnabled`.
12. **Enable autoread** — call `regmap_call` with method `enableAutoread`, arg1 `true`.
13. **Confirm autoread** — call `regmap_call` with method `isAutoreadEnabled`. Confirm `true`.
14. **Disable autoread** — call `regmap_call` with method `enableAutoread`, arg1 `false`.
15. **Register dump** — call `regmap_call` with method `registerDump`, arg1 `/tmp/scopy_regdump.csv`.

---

## Phase 9: Core API — Additional Methods

1. **Save setup** — call `scopy_call` with method `saveSetup`, arg1 `mcp_test_setup`.
2. **Load setup** — call `scopy_call` with method `loadSetup`, arg1 `mcp_test_setup`.
3. **Get tools for plugin** — call `scopy_call` with method `getToolsForPlugin`, arg1 `AD936XPlugin`.
4. **Run tool** — call `scopy_call` with method `runTool`, arg1 = a tool name, arg2 `true`. Then `false` to stop.

---

## Phase 10: Cleanup

1. **Disconnect device** — call `disconnect_device`. Confirm `true`.

---

## Summary

After completing all phases, compile a report:
- Total MCP tool calls made
- Number of successes vs failures  
- List any tools/methods that returned errors or unexpected values
- List any tools/methods that could NOT be tested and why
- Specific notes on:
  - Did `enableMonitor` (the fixed version) work correctly?
  - Did attach mode (no `start_scopy`) work?
  - Any AD936X parameters that failed to read/write?
  - Any IIO Explorer paths that didn't resolve?
