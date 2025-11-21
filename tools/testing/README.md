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

### 2. **Execute Tests Manually**
Edit the generated RST files in `testing_results/testing_results_v3.0.0/` and fill in:
- **Tested OS:** Your operating system
- **Comments:** Test execution notes
- **Result:** PASS or FAIL

### 3. **CSV Template Auto-Generated**
The CSV template is automatically created during step 1. You can also generate it manually:
```bash
python3 parseTestResults.py v3.0.0
```

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
- `--new-since` - Include tests added since specified git version

**Examples:**
```bash
# Copy all tests for v3.0.0
python3 setup_test_environment.py v3.0.0

# Only P0 and P1 priority tests
python3 setup_test_environment.py v3.0.0 --rbp P0 P1

# Only ADC and DAC component tests
python3 setup_test_environment.py v3.0.0 --component adc dac

# Combined filters: M2K component with P0 priority
python3 setup_test_environment.py v3.0.0 --component m2k --rbp P0

# Release testing: All P0 tests + new tests since v2.5.0
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0

# Only new features testing
python3 setup_test_environment.py v3.0.0 --new-since v2.5.0
```

**Output:**
- Creates `testing_results/testing_results_v3.0.0/` directory
- Contains filtered RST files ready for manual testing
- Preserves RST structure for documentation compilation

### `parseTestResults.py` - CSV Template Generation

**Purpose:** Generates CSV templates from testing directories for manual test execution tracking.

**Basic Usage:**
```bash
python3 parseTestResults.py <version> [OPTIONS]
```

**Parameters:**
- `version` - Version to parse (must match testing_results directory)
- `--output` - Custom output Excel file path (optional)

**Examples:**
```bash
# Parse v3.0.0 test results
python3 parseTestResults.py v3.0.0

# Custom output location
python3 parseTestResults.py v3.0.0 --output /path/to/custom_results.xlsx
```

**Output:**
- Creates `testing_results/testing_results_{version}.csv`
- Template with empty result fields for manual completion
- Columns: Test UID | Component | RBP | Result | Tested OS | Comments | Tester | File

### `generateTestsTable.py` - Source Documentation Tracking

**Purpose:** Generates Excel tracking tables directly from source documentation (before test execution).

**Basic Usage:**
```bash
python3 generateTestsTable.py <version> [OPTIONS]
```

**Parameters:**
- `version` - Version name for Excel worksheet
- `--sort` - Sort tests by RBP priority instead of UID
- `--filter` - Filter tests by RBP levels (e.g., P0,P1)

**Examples:**
```bash
# Generate tracking table for v3.0.0
python3 generateTestsTable.py v3.0.0

# Only P0 and P1 tests, sorted by priority
python3 generateTestsTable.py v3.0.0 --filter P0,P1 --sort
```

**Output:**
- Creates/updates `scopy_tests_tracking.xlsx` in scopy root
- Columns: Test UID | Test RBP | Test Pass/Fail (empty for manual entry)

## Complete Testing Workflow

### Release Testing Example (v3.0.0)

**1. Prepare Test Environment**
```bash
# Create comprehensive release testing environment
python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.5.0
```

**2. Execute Tests**
```bash
cd testing_results/testing_results_v3.0.0/
# Manually edit RST files with test results:
```

**Example test result format:**
```rst
**Tested OS:** Windows 11

**Comments:** Test passed successfully. Minor UI lag observed but within acceptable range.

**Result:** PASS
```

**3. Track Test Execution**
- Edit the auto-generated CSV template: `testing_results/testing_results_v3.0.0.csv`
- Fill in Result, Tested OS, Comments, and Tester columns for each test

**4. Review Results**
- Open `testing_results/testing_results_v3.0.0.csv`
- Use spreadsheet software for analysis and reporting


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



