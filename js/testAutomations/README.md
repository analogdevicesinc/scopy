# Scopy Automated Tests

## Running Scripts — Path Requirements

All test scripts use `evaluateFile()` with paths **relative to the Scopy build directory**. The `evaluateFile()` function resolves paths against the current working directory (CWD), not the script file's location.

Enable the `JS_TESTS_ENABLED` CMake option to copy the `js/` directory into the build directory, then run tests directly from the build directory:

```bash
cd scopy/build/
cmake .. -DENABLE_SCOPYJS=ON -DJS_TESTS_ENABLED=ON
make -j$(nproc)
./scopy --script js/testAutomations/<path-to-test>.js
```

For example:
```bash
cd scopy/build/
./scopy --script js/testAutomations/core/pluginLoadTests.js
./scopy --script js/testAutomations/m2k/voltmeter/voltmeter_dc_loopback.js
```

All test scripts load the shared test framework via:
```javascript
evaluateFile("js/testAutomations/common/testFramework.js");
```
This path resolves correctly from any build directory when `JS_TESTS_ENABLED=ON` is set.

---

## Requirements

- **Scopy** built with `-DENABLE_SCOPYJS=ON -DJS_TESTS_ENABLED=ON`
- **iio-emu** for emulator-based tests (`iio-emu adalm2000`, `iio-emu generic`, etc.)
- **ADALM2000 hardware** for M2K loopback tests requiring real signal paths

```bash
cd scopy/build/
cmake .. -DENABLE_SCOPYJS=ON -DJS_TESTS_ENABLED=ON
make -j$(nproc)
```

---

## Directory Structure

```
js/testAutomations/
├── common/                          # Shared framework and utilities
│   ├── testFramework.js
│   └── apiUnregisterTest.js
├── core/                            # Core Scopy functionality tests
│   ├── connectionTests.js
│   ├── emulatorTests.js
│   ├── packageManagerTests.js
│   ├── pluginLoadTests.js
│   └── preferencesTests.js
├── m2k/                             # ADALM2000 (M2K) tool tests
│   ├── oscilloscope/
│   │   └── osc_siggen_loopback.js
│   ├── signalGenerator/
│   │   └── signalGeneratorOscilloscopeTests.js
│   ├── voltmeter/
│   │   ├── voltmeter_dc_loopback.js
│   │   ├── voltmeter_dc_differential.js
│   │   └── voltmeter_dc_reversed.js
│   ├── powerSupply/
│   │   └── ps_voltmeter_loopback.js
│   ├── spectrumAnalyzer/
│   │   └── sa_loopback.js
│   ├── digitalIO/
│   │   ├── dio_loopback.js
│   │   ├── dio_voltmeter.js
│   │   └── dio_voltmeter_reversed.js
│   ├── networkAnalyzerDocTests.js
│   ├── networkAnalyzerVisualTests.js
│   ├── patternGeneratorDocTests.js
│   └── patternGeneratorVisualTests.js
├── generic-plugins/                 # Generic plugin tests
│   ├── registerMap/
│   │   ├── registerMapTests.js
│   │   └── registerMapVisualTests.js
│   ├── dataLogger/
│   │   ├── dataLoggerTests.js
│   │   └── dataLoggerVisualTests.js
│   └── debugger/
│       └── debuggerTests.js
├── ad9084/                          # AD9084 device plugin tests
│   ├── ad9084DocTests.js
│   └── ad9084VisualTests.js
├── ad936x/                          # AD936x / FMCOMMS5 device plugin tests
│   ├── ad936xDocTests.js
│   ├── ad936xVisualTests.js
│   ├── ad936xAdvancedTests.js
│   ├── fmcomms5DocTests.js
│   ├── fmcomms5VisualTests.js
│   └── fmcomms5AdvancedTests.js
├── adrv9002/                        # ADRV9002 device plugin tests
│   ├── adrv9002DocTests.js
│   └── adrv9002VisualTests.js
├── pqm/                             # PQM plugin tests
│   └── pqmTests.js
├── rfpowermeter/                    # RF Power Meter tests
│   ├── rfpowermeterDocTests.js
│   └── rfpowermeterVisualTests.js
└── swiot/                           # SWIOT plugin tests
    ├── swiotTests.js
    └── swiotSupervisedTests.js
```

---

## Test Types

Test files follow a naming convention that indicates what they validate:

- **`*Tests.js`** — Functional/API tests. Exercise Scopy APIs programmatically (read/write properties, call functions, verify return values). Can run unattended.
- **`*DocTests.js`** — Documentation/configuration tests. Validate that plugin settings, controls, and configurations match expected values. Typically run against an emulator.
- **`*VisualTests.js`** — Visual validation tests. Capture screenshots or require manual inspection to verify UI rendering. May need human supervision.
- **`*SupervisedTests.js`** — Tests that require active human observation during execution.

---

## Test File Reference

### Common

| File | Description |
|------|-------------|
| `common/testFramework.js` | Shared test framework loaded by all test files |
| `common/apiUnregisterTest.js` | Verifies APIs unregister correctly on disconnect |

### Core

| File | Description | Device |
|------|-------------|--------|
| `core/connectionTests.js` | Connection lifecycle (add/connect/disconnect/forget) | `ip:192.168.2.1` or emulator |
| `core/emulatorTests.js` | Emulator enable/disable for various device types | `ip:127.0.0.1` (requires iio-emu) |
| `core/pluginLoadTests.js` | Plugin detection and tool switching | `ip:192.168.2.1` + `ip:127.0.0.1` |
| `core/preferencesTests.js` | Save/load session preferences | None |
| `core/packageManagerTests.js` | Package install/uninstall | None (needs `test_package.zip` in fixtures/) |

### M2K Tests

All M2K tests require an ADALM2000 at `ip:192.168.2.1` (hardware) or `ip:127.0.0.1` (emulator) unless noted. Loopback tests require real hardware with the specified wiring.

| File | Description | Hardware Wiring |
|------|-------------|-----------------|
| `m2k/oscilloscope/osc_siggen_loopback.js` | Oscilloscope + signal generator loopback | W1->1+, W2->2+ |
| `m2k/signalGenerator/signalGeneratorOscilloscopeTests.js` | Signal generator + oscilloscope integration | W1->1+, W2->2+ |
| `m2k/voltmeter/voltmeter_dc_loopback.js` | Voltmeter DC loopback | V+->1+, V-->2+ |
| `m2k/voltmeter/voltmeter_dc_differential.js` | Voltmeter differential measurement | V+->1+, V-->1- |
| `m2k/voltmeter/voltmeter_dc_reversed.js` | Voltmeter reversed polarity | V-->1+, V+->2+ |
| `m2k/powerSupply/ps_voltmeter_loopback.js` | Power supply + voltmeter loopback | V+->1+, V-->2+ |
| `m2k/spectrumAnalyzer/sa_loopback.js` | Spectrum analyzer loopback | CH1+->W1, CH2+->W2 |
| `m2k/digitalIO/dio_loopback.js` | Digital I/O loopback | DIO 0-7->DIO 8-15 |
| `m2k/digitalIO/dio_voltmeter.js` | DIO + voltmeter integration | DIO 0->1+, DIO 8->V+ |
| `m2k/digitalIO/dio_voltmeter_reversed.js` | DIO + voltmeter reversed | DIO 8->1+, DIO 1->V+ |
| `m2k/networkAnalyzerDocTests.js` | Network analyzer configuration tests | Emulator |
| `m2k/networkAnalyzerVisualTests.js` | Network analyzer visual validation | Visual inspection |
| `m2k/patternGeneratorDocTests.js` | Pattern generator configuration tests | Emulator |
| `m2k/patternGeneratorVisualTests.js` | Pattern generator visual validation | Visual inspection |

### Generic Plugin Tests

| File | Description |
|------|-------------|
| `generic-plugins/registerMap/registerMapTests.js` | Register map read/write operations |
| `generic-plugins/registerMap/registerMapVisualTests.js` | Register map visual validation |
| `generic-plugins/dataLogger/dataLoggerTests.js` | Data logger single-tool tests |
| `generic-plugins/dataLogger/dataLoggerVisualTests.js` | Data logger visual validation |
| `generic-plugins/debugger/debuggerTests.js` | Debugger explorer and code generation |

### Device Plugin Tests

#### AD9084

| File | Description | Device |
|------|-------------|--------|
| `ad9084/ad9084DocTests.js` | AD9084 configuration documentation tests | `ip:127.0.0.0` |
| `ad9084/ad9084VisualTests.js` | AD9084 visual validation | `ip:127.0.0.0` |

#### AD936x / FMCOMMS5

| File | Description | Device |
|------|-------------|--------|
| `ad936x/ad936xDocTests.js` | AD936x configuration documentation tests | `ip:192.168.2.1` |
| `ad936x/ad936xVisualTests.js` | AD936x visual validation | `ip:192.168.2.1` |
| `ad936x/ad936xAdvancedTests.js` | AD936x advanced plugin tests | `ip:192.168.2.1` |
| `ad936x/fmcomms5DocTests.js` | FMCOMMS5 configuration documentation tests | `ip:127.0.0.0` |
| `ad936x/fmcomms5VisualTests.js` | FMCOMMS5 visual validation | `ip:127.0.0.0` |
| `ad936x/fmcomms5AdvancedTests.js` | FMCOMMS5 advanced plugin tests | `ip:127.0.0.0` |

#### ADRV9002

| File | Description | Device |
|------|-------------|--------|
| `adrv9002/adrv9002DocTests.js` | ADRV9002 configuration documentation tests | `ip:127.0.0.0` |
| `adrv9002/adrv9002VisualTests.js` | ADRV9002 visual validation | `ip:127.0.0.0` |

#### PQM

| File | Description | Device |
|------|-------------|--------|
| `pqm/pqmTests.js` | PQM plugin tests (power quality monitoring) | `ip:127.0.0.1` |

#### RF Power Meter

| File | Description | Device |
|------|-------------|--------|
| `rfpowermeter/rfpowermeterDocTests.js` | RF Power Meter documentation tests | Requires `powrms` IIO device |
| `rfpowermeter/rfpowermeterVisualTests.js` | RF Power Meter visual validation | Requires `powrms` IIO device |

#### SWIOT

| File | Description | Device |
|------|-------------|--------|
| `swiot/swiotTests.js` | SWIOT plugin tests (Config, AD74413R, MAX14906, Faults) | `ip:127.0.0.1` |
| `swiot/swiotSupervisedTests.js` | SWIOT supervised tests (requires visual supervision) | `ip:127.0.0.1` |
