# Scopy Testing Tools

A comprehensive set of Python tools for managing Scopy's manual testing workflow. These tools automate test environment setup, result parsing, and CSV template generation for release testing processes.

## Quick Start

### 1. **Generate Test Environment**
```bash
# Create test environment for v3.0.0 with all tests
python3 setup_test_environment.py v3.0.0

# Filter by component
python3 setup_test_environment.py v3.0.0 --component adc dac

# Filter by priority level
python3 setup_test_environment.py v3.0.0 --rbp P0 P1

# Release testing: P0 tests + new features since last version
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0
```
### 2. **CSV Template Auto-Generated**
The CSV template is automatically created during step 1. You can also generate it manually:
```bash
python3 parseTestResults.py v3.0.0
```

### 3. **Execute Tests Manually**
Edit the generated csv file to add test results based on provided columns


## Detailed Usage

### `setup_test_environment.py` - Test Environment Setup

**Purpose:** Creates filtered copies of test documentation for manual testing execution.

**Basic Usage:**
```bash
python3 setup_test_environment.py <version> [OPTIONS]
```

**Parameters:**
- `version` - Version name for the test directory (e.g., v3.0.0)
- `--rbp` - Filter by RBP priority levels (P0, P1, P2, P3)
- `--component` - Filter by component names (adc, dac, m2k, core, etc.)
- `--new-since` - Include tests added since specified git version (overrides all other filters so that all new tests are added)

**Available Components:**
Components are determined by folder structure in `docs/tests/`:
- **Plugin components:** Any subdirectory in `docs/tests/plugins/` (e.g., adc, dac, m2k, ad936x, ad9084, adrv9002, fmcomms5, swiot1l, datalogger, debugger, jesd, pqm, registermap)
- **Core component:** Files in `docs/tests/general/core/` subdirectory
- **General meta-component:** Using `--component general` includes ALL files from the `general/` directory, including both core files and other general test files

**Filter Logic:**
- When using `--component` and/or `--rbp` without `--new-since`: Tests must match ALL specified filters (AND logic)
- When using `--new-since`: Filter logic becomes `(component AND rbp) OR new-since`
  - This means all new tests are included regardless of component/rbp filters
  - Plus any existing tests matching the component/rbp criteria

**Examples:**
```bash
# Copy all tests for v3.0.0
python3 setup_test_environment.py v3.0.0

# Only P0 and P1 priority tests
python3 setup_test_environment.py v3.0.0 --rbp P0 P1

# Only ADC and DAC component tests
python3 setup_test_environment.py v3.0.0 --component adc dac

# Core component tests only
python3 setup_test_environment.py v3.0.0 --component core

# All general tests (includes core and all other general/ files)
python3 setup_test_environment.py v3.0.0 --component general

# Combined filters: M2K component with P0 priority
python3 setup_test_environment.py v3.0.0 --component m2k --rbp P0

# Release testing: All P0 tests + All new tests since v2.5.0
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0

# Only new features testing
python3 setup_test_environment.py v3.0.0 --new-since v2.5.0
```

**Output:**
- Creates `testing_results/testing_results_v3.0.0/` directory
- Contains filtered RST files ready for manual testing
- Automatically generates CSV template
- If directory exists, prompts to overwrite (this will also delete and regenerate the CSV file)

### `parseTestResults.py` - CSV Template Generation

**Purpose:** Generates CSV templates from testing directories for manual test execution tracking.

**Basic Usage:**
```bash
python3 parseTestResults.py <version> [OPTIONS]
```

**Parameters:**
- `version` - Version to parse (must match testing_results directory)
- `--output` - Custom output CSV file path (optional)
- `--force` - Force overwrite without prompting (optional)

**Examples:**
```bash
# Parse v3.0.0 test results
python3 parseTestResults.py v3.0.0

# Custom output location
python3 parseTestResults.py v3.0.0 --output /path/to/custom_results.csv

# Force overwrite without prompting
python3 parseTestResults.py v3.0.0 --force
```

**Output:**
- Creates `testing_results/testing_results_{version}.csv`
- Template with empty result fields for manual completion
- Columns: Test UID | Component | RBP | Result | Tested OS | Comments | Tester | File

## Complete Testing Workflow

### Release Testing Example (v3.0.0)

**1. Prepare Test Environment**
```bash
# Create comprehensive release testing environment
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0
```

**2. Execute Tests and Track Results**
- Open the auto-generated CSV template: `testing_results/testing_results_v3.0.0.csv`
- For each test, fill in:
  - **Result:** PASS, FAIL, or SKIP
  - **Tested OS:** Operating system used
  - **Comments:** Any observations or notes
  - **Tester:** Your name or identifier

**3. Review and Analyze**
- Save the completed CSV file
- Use spreadsheet software for analysis and reporting
- Historical data is preserved across versions



##  Requirements

### Git Requirements
- Git repository (required for `--new-since` functionality)
- Proper git tags for version references

### File Structure Requirements
```
scopy/
├── docs/tests/           # Source test documentation
├── testing_results/      # Generated test environments
└── tools/testing/        # This toolset
```

## CSV Template Format

### Columns
- **Test UID** - Unique test identifier
- **Component** - Test component (adc, dac, m2k, core, etc.)
- **RBP** - Risk-based priority (P0, P1, P2, P3)
- **Result** - Test outcome (empty template field for PASSED/FAILED/SKIPPED)
- **Tested OS** - Operating system used for testing (empty template field)
- **Comments** - Test execution notes (empty template field)
- **Tester** - Person executing the tests (empty template field)
- **File** - Source RST filename

### Multiple Versions
- Each version gets its own CSV file: `testing_results_{version}.csv`
- Historical test data preserved across versions
- Easy import into spreadsheet tools for analysis

## Troubleshooting

### Common Issues

**"Git tag not found" error:**
```bash
# Check available git tags
git tag -l

# Use exact tag name
python3 setup_test_environment.py v3.0.0 --new-since v2.5.0
```

**"Not in a git repository" error:**
```bash
# Ensure you're in the scopy git repository root
cd /path/to/scopy
python3 tools/testing/setup_test_environment.py v3.0.0 --new-since v2.5.0
```

**No tests found:**
- Verify `docs/tests/` directory exists
- Check test files contain proper `**UID:**` and `**RBP:**` metadata



