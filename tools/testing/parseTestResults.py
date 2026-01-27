#!/usr/bin/env python3
"""
Generate CSV templates from testing_results directories for manual test execution.
Creates version-specific CSV files with test metadata for tracking test execution.

Usage:
    python3 parseTestResults.py v3.0.0                    # Generate v3.0.0 CSV template
    python3 parseTestResults.py v3.0.0 --output custom.csv  # Custom output file
"""

import os
import sys
import re
import argparse
import csv
from pathlib import Path


def build_testing_results_path(version):
    """Build testing results path from version string."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    scopy_root = os.path.dirname(os.path.dirname(script_dir))  # tools/testing/ → scopy/
    return os.path.join(scopy_root, "testing_results", f"testing_results_{version}")


def get_output_csv_path(version, custom_path=None):
    """Get CSV output path (default: testing_results/testing_results_v{version}.csv)."""
    if custom_path:
        return custom_path

    script_dir = os.path.dirname(os.path.abspath(__file__))
    scopy_root = os.path.dirname(os.path.dirname(script_dir))  # tools/testing/ → scopy/
    return os.path.join(scopy_root, "testing_results", f"testing_results_{version}.csv")


def scan_testing_results_files(results_dir):
    """Find all RST files in testing results directory."""
    rst_files = []

    for root, dirs, files in os.walk(results_dir):
        for file in files:
            if file.endswith('.rst'):
                file_path = os.path.join(root, file)
                relative_path = os.path.relpath(file_path, results_dir)
                component = extract_component_from_testing_path(relative_path)

                rst_files.append({
                    'file_path': file_path,
                    'relative_path': relative_path,
                    'component': component,
                    'filename': file
                })

    return rst_files


def extract_component_from_testing_path(relative_path):
    """Extract component from testing results file path."""
    parts = relative_path.split('/')

    if parts[0] == 'plugins' and len(parts) >= 2:
        return parts[1]  # plugins/adc/... → 'adc'
    elif parts[0] == 'general':
        if len(parts) >= 2 and parts[1] == 'core':
            return 'core'
        elif 'preferences' in parts[-1]:
            return 'preferences'
        elif 'package_manager' in parts[-1]:
            return 'package_manager'
        elif 'scripting' in parts[-1]:
            return 'scripting'
        elif 'instrument_detaching' in parts[-1]:
            return 'instrument_detaching'
        else:
            return 'general'
    else:
        return 'other'


def parse_rst_structure(content):
    """Parse RST into header section + individual test sections."""
    # Find test boundaries using pattern: "Test N " followed by underline
    test_pattern = r'^(Test \d+[^\n]*\n[\^=\-~]+\n)'

    # Split content at test boundaries
    parts = re.split(test_pattern, content, flags=re.MULTILINE)

    if len(parts) == 1:
        # No tests found
        return {'header': content, 'tests': []}

    header = parts[0]  # Everything before first test
    tests = []

    # Process test sections 
    for i in range(1, len(parts), 2):
        if i + 1 < len(parts):
            heading = parts[i]
            content_block = parts[i + 1]

            # Extract UID/RBP from content
            uid = extract_uid_from_content(content_block)
            rbp = extract_rbp_from_content(content_block)

            tests.append({
                'heading': heading,
                'content': content_block,
                'uid': uid,
                'rbp': rbp
            })

    return {'header': header, 'tests': tests}


def extract_uid_from_content(content):
    """Extract UID from test content block."""
    # Handle both **UID:** and **UID**: formats
    uid_pattern = r'\*\*UID[\*:]+\s*([A-Z_][A-Z0-9_.]*)'
    match = re.search(uid_pattern, content)
    return match.group(1) if match else 'MISSING'


def extract_rbp_from_content(content):
    """Extract RBP from test content block."""
    rbp_pattern = r'\*\*RBP:\*\*\s+(P[0-3])'
    match = re.search(rbp_pattern, content)
    return match.group(1) if match else 'MISSING'




def parse_test_metadata_from_rst(file_path):
    """Parse RST file and extract test metadata (UID, RBP only)."""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        structure = parse_rst_structure(content)
        return structure['tests']

    except Exception as e:
        print(f"Warning: Error parsing {file_path}: {e}")
        return []


def process_all_test_data(results_dir):
    """Main processing function - extract test metadata from directory."""
    all_test_data = []

    rst_files = scan_testing_results_files(results_dir)

    for rst_file in rst_files:
        # Skip index files - they don't contain tests
        if 'index.rst' in rst_file['filename']:
            continue

        tests = parse_test_metadata_from_rst(rst_file['file_path'])

        for test in tests:
            test_data = {
                'uid': test['uid'],
                'component': rst_file['component'],
                'rbp': test['rbp'],
                'result': '',
                'tested_os': '',
                'comments': '',
                'tester': '',
                'file': rst_file['filename']
            }
            all_test_data.append(test_data)

    return all_test_data


def validate_and_fill_missing_data(test_data):
    """Ensure all fields have values (empty strings for missing)."""
    for test in test_data:
        for key, value in test.items():
            if value is None :
                test[key] = ''

    return test_data




def generate_csv(test_data, csv_path, force=False):
    """Generate CSV file with test data template."""

    # Check if file exists and prompt user (unless force is True)
    if os.path.exists(csv_path) and not force:
        response = input(f"File {csv_path} exists. Overwrite? (y/N): ")
        if response.lower() != 'y':
            print("Aborted.")
            return False
      

    # CSV file header 
    headers = ['Test UID', 'Component', 'RBP', 'Result', 'Tested OS', 'Comments', 'Tester', 'File']

    with open(csv_path, 'w', newline='', encoding='utf-8') as csvfile:
        writer = csv.writer(csvfile)

        # Write header row
        writer.writerow(headers)

        # Write test data rows
        for test in test_data:
            writer.writerow([
                test['uid'],
                test['component'],
                test['rbp'],
                test['result'],
                test['tested_os'],
                test['comments'],
                test['tester'],
                test['file']
            ])

    return True

def main():
    parser = argparse.ArgumentParser(
        description='Generate CSV template from Scopy test metadata',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument('version', help='Version to process (e.g., v3.0.0)')
    parser.add_argument('--output', help='Custom output CSV file path')
    parser.add_argument('--force', action='store_true', help='Force overwrite without prompting')

    args = parser.parse_args()

    # Build paths
    results_dir = build_testing_results_path(args.version)
    csv_path = get_output_csv_path(args.version, args.output)

    # Validate input directory exists
    if not os.path.exists(results_dir):
        print(f"Error: Testing results directory not found: {results_dir}")
        print(f"Make sure you've run setup_test_environment.py for version {args.version}")
        sys.exit(1)

    # Create output directory if needed
    output_dir = os.path.dirname(csv_path)
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Process all test data
    test_data = process_all_test_data(results_dir)
    test_data = validate_and_fill_missing_data(test_data)

    if not test_data:
        print("Warning: No test data found in the specified directory.")
        print("Make sure the directory contains RST files with test metadata.")
        sys.exit(1)

    # Generate CSV report
    success = generate_csv(test_data, csv_path, args.force)

    if not success:
        sys.exit(1)

    print("CSV generated successfully")


if __name__ == "__main__":
    main()