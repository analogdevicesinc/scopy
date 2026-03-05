# ADC Plugin Test Automation

Automated tests for the ADC plugin, based on the manual test documentation at `docs/tests/plugins/adc/adc_tests.rst`.

## Requirements

- Scopy built with ADC plugin enabled
- Pluto device connected via USB (`ip:192.168.2.1`) or emulator (`ip:127.0.0.1`)
- For genalyzer tests: Genalyzer library installed, signal generator setup (see `Pluto.signalGenerator` in test docs)

## Running

```bash
# Fully automated tests (no human interaction needed)
./scopy --script ../js/testAutomations/adc/adcDocTests.js 

# Visual validation tests (requires human observer, 3s pauses between steps)
./scopy --script ../js/testAutomations/adc/adcVisualTests.js 
```

## adcDocTests.js — 20 Automated Tests

Tests that run unattended with pass/fail results.

### ADC-Time

| UID | Description |
|-----|-------------|
| TST.ADC_TIME.X_AXIS | Buffer size, sync toggle, plot size, rolling mode |
| TST.ADC_TIME.Y_AXIS | Channel YMode (ADC Counts), YMin/YMax range |
| TST.ADC_TIME_CHANNEL_SETTINGS | Per-channel Y-axis with isolation check |
| TST.MEASURE | Enable measurements, run, verify numeric values |
| TST.ADC_TIME.XY_PLOT | Enable XY plot, set/verify source channel |
| TST.ADC_TIME.SETTINGS | Plot title, labels toggle, thickness, style via Plot API |
| TST.ADC_TIME.PRINT | Print plot to PDF via `printPlot()` |
| TST.CURSORS | X/Y cursor enable, position, lock, tracking via Plot API |

### ADC-Frequency

| UID | Description |
|-----|-------------|
| TST.ADC_FREQ.X_AXIS_SETTINGS | Buffer size, frequency offset |
| TST.ADC_FREQ.Y_AXIS_SETTINGS | Y range, power offset, window functions |
| TST.ADC_FREQ.MARKER_SETTINGS | Peak/fixed markers, count 5 and 7 |
| TST.ADC_FREQ.SETTINGS_ADJUSTMENT | Freq plot title, labels, thickness, style |
| TST.ADC_FREQ.PRINT_PLOT | Freq plot print to PDF |
| TST.ADC_FREQ.Y_AXIS_CHANNEL_SETTINGS | Per-channel freq Y range with isolation check |
| TST.ADC_FREQ.CURSOR_SETTINGS | Freq cursor enable, position, tracking |
| TST.ADC_FREQ.CHANNEL_AVERAGING | Enable averaging, set size to 10 |

### Genalyzer

Genalyzer tests enable complex mode automatically and restore it after.

| UID | Description |
|-----|-------------|
| TST.ADC_GENALYZER.ENABLE | Enable genalyzer, run, verify SNR metric |
| TST.ADC_GENALYZER.AUTO_MODE | Auto mode, SSB width set/verify (120, 60) |
| TST.ADC_GENALYZER.FIXED_TONE_MODE | Fixed tone mode, expected freq, harmonic order, SSB params |
| TST.ADC_GENALYZER.FFT_INTEGRATION | Averaging + window function + genalyzer combined |

## adcVisualTests.js — 17 Visual Tests

Same steps as DocTests but with `VISUAL CHECK:` log markers and 3-second pauses for human observation. Covers all DocTests except the two print tests (PRINT, PRINT_PLOT) which don't require visual verification.

## Not Automatable — 9 Tests

| UID | Reason |
|-----|--------|
| TST.ADC_TIME.INFO | Opens external browser |
| TST.PLOT_NAVIGATION | Requires mouse scroll, click-drag, shift+scroll |
| TST.ADC_FREQ.VIEW_PLUGIN_DOC | Opens external browser |
| TST.PREF.X_SCALE_POS | Requires app restart |
| TST.PREF.Y_SCALE_POS | Requires app restart |
| TST.PREF.CH_HANDLE_POS | Requires app restart |
| TST.PREF.X_CURSOR_POS | Requires app restart |
| TST.PREF.Y_CURSOR_POS | Requires app restart |
| TST.PREF.DEFAULT_YMODE | Requires app restart |
