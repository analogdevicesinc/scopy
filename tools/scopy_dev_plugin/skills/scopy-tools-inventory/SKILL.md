---
name: scopy-tools-inventory
description: Catalog of all Scopy development tools including package generator, testing tools, CI scripts, format/license scripts, and dev plugin commands. Loaded when analyzing what tools exist for a task.
---

# Scopy Tools Inventory

## Package Generator

**Location:** `tools/packagegenerator/`
**Purpose:** Generate complete package/plugin scaffolding from JSON configuration.

**Usage:**
```bash
cd tools/packagegenerator
pip install -r requirements.txt
./package_generator.py --all                    # Generate complete package
./package_generator.py -p /path/to/plugin.json  # Generate plugin only
./package_generator.py --init                   # Initialize git submodule
./package_generator.py --style                  # Add style templates
./package_generator.py --translation            # Add translation templates
./package_generator.py -a --src /path --dest /path  # Archive packages
```

**Input files:**
- `pkg.json` — Package metadata (id, title, description, version, category)
- `plugin.json` — Plugin structure definition with 14 fields:
  - `baseplugin`, `plugin_name`, `plugin_display_name`, `plugin_description`
  - `class_name`, `namespace`, `device_category`
  - `tools[]` — array of {id, tool_name, file_name, class_name, namespace}
  - `cmakelists` — {cmake_min_required, cxx_standard, enable_testing}
  - `style`, `test`, `resources`, `doc`, `pdk`

**When to use:** Creating a brand new package or plugin from scratch. Not for modifying existing plugins.

## Testing Tools

### Manual Test Environment
**Location:** `tools/testing/`

```bash
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0
```

- `setup_test_environment.py` — Create filtered test environments by version/component/priority
- `parseTestResults.py` — Generate CSV templates from RST test documentation
- `rst_parser.py` — Parse RST test files
- `file_filters.py` — Component and priority filtering

### JS Test Automation
**Location:** `js/testAutomations/`

```
testAutomations/
├── common/testFramework.js    # Base test API
├── core/                      # Core tests
├── generic-plugins/           # Generic plugin tests (adc/, dac/, etc.)
├── m2k/                       # M2K-specific tests
├── ad936x/, adrv9002/, ...    # Device-specific tests
```

- `<plugin>DocTests.js` — Category A: fully automatable tests
- `<plugin>VisualTests.js` — Category B: supervised visual checks
- Test names must match RST UIDs exactly
- Every setter followed by `msleep(500)` for hardware settle
- Save/restore original values in all code paths

### JS Hardware-in-the-Loop Testing
**Branch:** `claude_scopy_dev_plugin` (not yet in main)
**Purpose:** Automated testing with real hardware via JS scripts.

### Unit Tests
- Each library/plugin has `test/` subdirectory with Qt Test-based tests
- Enable with `-DENABLE_TESTING=ON`
- Run: `cd build && ctest -R <test_name>`

## Code Quality Tools

### Format Script
**Location:** `tools/format.sh`
- Uses clang-format for C++ files
- Uses cmake-format for CMake files
- Run before every commit

### License Script
**Location:** `tools/license.sh`
- Adds GPL license headers to source files
- Run before every commit

### Dev Plugin Quality Commands
- `/scopy_dev_plugin:check-code-quality <package>` — Static code quality analysis
- `/scopy_dev_plugin:verify-package <package>` — CI pre-flight validation
- `/scopy_dev_plugin:validate-api <plugin>` — Validate API class (checks A1-A7)
- `/scopy_dev_plugin:validate-automated-tests <plugin>` — Validate JS tests (checks T1-T7)

## Code Generation Commands

- `/scopy_dev_plugin:create-api <plugin>` — Generate Q_INVOKABLE API class for JS automation
- `/scopy_dev_plugin:create-docs <plugin>` — Generate RST user-guide documentation
- `/scopy_dev_plugin:create-test-docs <plugin>` — Generate RST test case documentation
- `/scopy_dev_plugin:create-automated-tests <plugin>` — Create JS automated test scripts
- `/scopy_dev_plugin:create-whatsnew <plugin> <version>` — Generate What's New HTML

## CI Scripts

**Location:** `ci/`

Platform-specific build scripts:
- `ci/ubuntu/` — Ubuntu/Linux build and packaging
- `ci/windows/` — Windows build with MSVC
- `ci/macOS/` — macOS build
- `ci/arm/` — ARM cross-compilation (armv7)
- `ci/android/` — Android build

Each directory contains scripts for dependency installation, build configuration, and packaging.

## MCP Server

**Location:** `tools/scopy-mcp-server/`
**Purpose:** External tooling integration via Model Context Protocol.

## Build System

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo
make -j$(nproc)

# Key flags
-DENABLE_ALL_PACKAGES=ON           # All packages
-DENABLE_PACKAGE_<NAME>=ON         # Specific package (uppercase)
-DENABLE_TESTING=ON                # Unit tests
-DENABLE_AUTOMATED_TESTS=ON        # JS automated tests (ctest)
-DSCOPY_DEV_MODE=ON                # Dev mode (clears cache on reconfigure)
```
