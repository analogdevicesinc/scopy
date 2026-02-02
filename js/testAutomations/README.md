# Scopy Automated Tests Documentation

## 📋 Overview

This directory contains automated JavaScript test suites for Scopy's various tools and plugins. These tests validate functionality, API behavior, and cross-tool integration using Scopy's JavaScript scripting interface.

## 🔌 General Requirements

### Software Requirements
- **Scopy**: Built with scripting support (`-DENABLE_SCOPYJS=ON`)
- **IIO Emulator** or **ADALM2000 (M2K)** hardware device
- **Operating System**: Linux, Windows, or macOS

### Build Scopy with Scripting Support
```bash
cd scopy/build/
cmake .. -DENABLE_SCOPYJS=ON
make -j$(nproc)
```

### Device Setup Options

#### Option 1: IIO Emulator (Recommended for Testing)
```bash
# Start the M2K emulator
iio-emu adalm2000

# The emulator will be available at ip:127.0.0.0
```

#### Option 2: Real ADALM2000 Hardware
- Connect M2K device via USB
- Device will be available at `ip:192.168.2.1` (default) or custom URI

---

## 🧪 Test Suites

### 1. M2K Tool Tests

#### 📊 **Signal Generator Tests**
**File:** `m2k/signalGenerator/signalGeneratorTests.js`

**Hardware Setup:**
- M2K Channel 1 Output (W1) → Oscilloscope Channel 1 Input (1+)
- M2K Channel 2 Output (W2) → Oscilloscope Channel 2 Input (2+)
- Connect grounds (1-, 2-) to GND

**Run Command:**
```bash
cd scopy/build/
./scopy --script ../js/testAutomations/m2k/signalGenerator/signalGeneratorTests.js
```

**Tests Coverage:**
- Sine wave generation only
- Frequency range (0.001Hz to 5MHz)
- Amplitude control (1mV to 10V)
- Phase adjustment (0-360°)
- Offset control (-2V to +2V)

---

#### 🔬 **Signal Generator + Oscilloscope Integration Tests**
**File:** `m2k/signalGenerator/signalGeneratorOscilloscopeTests.js`

**Hardware Setup:**
- Same as Signal Generator Tests (loopback connections)
- W1 → 1+, W2 → 2+, grounds connected

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/signalGenerator/signalGeneratorOscilloscopeTests.js
```

**Tests Coverage:**
- Constant voltage generation (-5V to +5V)
- Waveform generation (sine, square, triangle, sawtooth/ramp)
- Square wave duty cycle adjustment
- Dual-channel synchronization
- Cross-validation with oscilloscope measurements
- Measurement accuracy verification

---

#### 🔋 **Power Supply + Voltmeter Tests**
**File:** `m2k/powerSupply/powerSupplyVoltmeterTests.js`

**Hardware Setup:**
- M2K V+ Output → Voltmeter Channel 1 Input (1+)
- M2K V- Output → Voltmeter Channel 2 Input (2+)
- Connect grounds to GND

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/powerSupply/powerSupplyVoltmeterTests.js
```

**Tests Coverage:**
- Positive voltage output (V+) accuracy (0V to +5V)
- Negative voltage output (V-) accuracy (-5V to 0V)
- Fine voltage tuning with incremental steps
- Tracking mode validation (positive and negative)
- Dual-channel independent control
- Voltage limit verification
- Enable/disable channel control

---

#### 📏 **Voltmeter Tests**
**File:** `m2k/voltmeter/voltmeterTests.js`

**Hardware Setup:**
- External voltage source → Channel 1 (1+) and/or Channel 2 (2+)
- Or use M2K power supply outputs for self-test
- Connect grounds appropriately

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/voltmeter/voltmeterTests.js
```

**Tests Coverage:**
- Mode switching (DC/AC low/AC high)
- Start/stop control functionality

---

#### 📏 **Voltmeter + Power Supply Integration Tests**
**File:** `m2k/voltmeter/voltmeterPowerSupplyTests.js`

**Hardware Setup:**
- V+ → 1+, V- → 2+ (loopback for validation)
- Grounds connected

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/voltmeter/voltmeterPowerSupplyTests.js
```

**Tests Coverage:**
- DC voltage measurement (Channel 1 and Channel 2)
- Dual-channel simultaneous measurement
- Peak hold functionality
- Data logging features with CSV export

---

#### 📏 **Voltmeter + Signal Generator Integration Tests**
**File:** `m2k/voltmeter/voltmeterSignalGeneratorTests.js`

**Hardware Setup:**
- W1 → 1+, W2 → 2+ (signal validation)
- Grounds connected

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/voltmeter/voltmeterSignalGeneratorTests.js
```

**Tests Coverage:**
- AC low frequency measurement (20Hz-8kHz)
- AC high frequency measurement (8kHz-40kHz)
- Histogram mode testing with statistical analysis

---

#### 🔌 **Digital I/O Tests**
**File:** `m2k/digitalIO/digitalIOTests.js`

**Hardware Setup Options:**

**Option A - Loopback Testing (Recommended):**
- DIO 0 → DIO 8
- DIO 1 → DIO 9
- DIO 2 → DIO 10
- DIO 3 → DIO 11
- DIO 4 → DIO 12
- DIO 5 → DIO 13
- DIO 6 → DIO 14
- DIO 7 → DIO 15

**Option B - External Testing:**
- Connect LEDs with current-limiting resistors to DIO pins
- Or connect to external digital circuits

**Run Command:**
```bash
./scopy --script ../js/testAutomations/m2k/digitalIO/digitalIOTests.js
```

**Tests Coverage:**
- Pin direction configuration (input/output for pins 0-15)
- Output control (high/low states)
- Input reading validation with loopback
- Binary counter pattern generation
- Walking ones/zeros patterns
- Rapid toggle stress testing
- Grouped pin operations
- Start/stop control

---

### 2. Generic Plugin Tests

#### 📋 **Register Map Tests**
**File:** `generic-plugins/registerMap/registerMapTests.js`

**Hardware Setup:**
- Any IIO device with register map support
- M2K device works for basic testing

**Run Command:**
```bash
./scopy --script ../js/testAutomations/generic-plugins/registerMap/registerMapTests.js
```

**Tests Coverage:**
- Device enumeration and selection
- Register read/write operations
- Multiple register operations
- Bit field manipulation
- Register search functionality
- Auto-read toggle
- Register dump functionality
- Device switching between multiple IIO devices
- Stress testing with rapid operations

---

#### 📊 **Data Logger Tests**
**File:** `generic-plugins/dataLogger/dataLoggerTests.js`

**Hardware Setup:**
- Any IIO device with monitor support
- M2K with active measurements recommended

**Run Command:**
```bash
./scopy --script ../js/testAutomations/generic-plugins/dataLogger/dataLoggerTests.js
```

**Tests Coverage:**
- Monitor listing and device enumeration
- Monitor enabling/disabling
- Start/stop control
- Tool creation from monitors
- Single-shot data logging
- Continuous data logging
- Multi-tool simultaneous logging
- Monitor configuration and management

---

### 3. Master Test Runner

#### 🔄 **Run All Tests**
**File:** `runAllTests.js`

**Hardware Setup:**
- Complete loopback setup as described above:
  - W1 → 1+, W2 → 2+
  - V+ → 1+, V- → 2+
  - DIO 0-7 → DIO 8-15
  - All grounds connected

**Run Command:**
```bash
./scopy --script ../js/testAutomations/runAllTests.js
```

**Note:** This runs all test suites sequentially. Ensure proper hardware connections for all tests.

---

## 🔧 Hardware Connection Diagrams

### Basic M2K Loopback Setup
```
ADALM2000 (M2K) Device
┌─────────────────────┐
│  Signal Generator   │
│  W1 ──────────┐    │
│  W2 ────────┐ │    │
│             │ │    │
│  Oscilloscope │ │   │
│  1+ ◄────────┘ │   │
│  2+ ◄──────────┘   │
│  1- ──┐            │
│  2- ──┼── GND      │
│       │            │
│  Power Supply      │
│  V+ ──────────┐    │
│  V- ────────┐ │    │
│             │ │    │
│  Digital I/O │ │    │
│  DIO0 ──► DIO8 │   │
│  DIO1 ──► DIO9 │   │
│  ...          │    │
│  DIO7 ──► DIO15    │
└─────────────────────┘
```

---

## 📝 Test Output Format

All tests follow a consistent output format:

```
========================================
    TEST SUITE: [Tool Name] Tests
========================================
Connecting to device: ip:127.0.0.1...
✅ Device connected successfully

Test 1: [Test Name]
----------------------------------------
[Test steps and validations]
✅ PASS: [Success message]

Test 2: [Test Name]
----------------------------------------
[Test steps and validations]
❌ FAIL: [Failure reason]

========================================
    RESULTS SUMMARY
========================================
Total Tests: X
Passed: Y (Z%)
Failed: A (B%)
========================================
```

---

## 🚀 Quick Start Guide

### 1. Minimal Testing (Emulator Only)
```bash
# Start emulator
iio-emu adalm2000

# Run basic connectivity test
cd scopy/build/
./scopy --script ../js/testAutomations/m2k/signalGenerator/signalGeneratorTests.js
```

### 2. Comprehensive Testing (Real Hardware)
```bash
# Connect M2K device with full loopback setup
# Run all tests
cd scopy/build/
./scopy --script ../js/testAutomations/runAllTests.js
```

### 3. Specific Tool Testing
```bash
# Test individual tools as needed
./scopy --script ../js/testAutomations/m2k/voltmeter/voltmeterTests.js
./scopy --script ../js/testAutomations/m2k/digitalIO/digitalIOTests.js
```

---

## ⚠️ Important Notes

1. **Device URI**: Tests default to `ip:127.0.0.1` for emulator. Modify the connection string in test files for real hardware.

2. **Timing**: Tests include delays (`msleep()`) for hardware synchronization. Do not reduce these without testing.

3. **Cleanup**: Tests attempt to restore initial state. However, manual device reset may be needed between test runs.

4. **Error Handling**: Tests include try-catch blocks. Check console output for detailed error messages.

5. **Hardware Damage**: Ensure proper connections to avoid hardware damage. Never exceed voltage ratings.

---

## 🐛 Troubleshooting

### Common Issues and Solutions

| Issue | Solution |
|-------|----------|
| "Connection refused" | Ensure IIO emulator is running or device is connected |
| "Device not found" | Check device URI and USB connection |
| "Measurement timeout" | Increase delay times in test scripts |
| "0V readings" | Verify loopback connections and ground references |
| "IIO Error -9" | Update to latest Scopy version with API fixes |

### Debug Mode
Enable verbose output by modifying test files:
```javascript
const DEBUG = true;  // Set to true for detailed logging
```

---

## 📚 Additional Resources

- [Scopy Documentation](https://github.com/analogdevicesinc/scopy)
- [ADALM2000 Wiki](https://wiki.analog.com/university/tools/m2k)
- [IIO Emulator Documentation](https://github.com/analogdevicesinc/iio-emu)
- [Test Development Guide](../../../docs/SCOPY_TEST_AUTOMATION_PLAN.md)

---

## 👥 Contributing

To add new tests:
1. Follow the existing test structure pattern
2. Use the common test framework (`common/testFramework.js`)
3. Document hardware requirements clearly
4. Include both positive and negative test cases
5. Update this README with new test information

---

## 📄 License

These test suites are part of the Scopy project and follow the same licensing terms.

---

*Last Updated: February 2, 2026*
*Version: 1.0.0*