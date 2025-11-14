#!/usr/bin/env python3
"""
Test Environment Setup Script
Creates filtered copies of test documentation for manual testing execution.

Usage:
    python3 setup_test_environment.py v3.0.0                          # Copy all tests
    python3 setup_test_environment.py v3.0.0 --rbp P0 P1              # Copy P0/P1 tests only
    python3 setup_test_environment.py v3.0.0 --component adc dac      # Copy adc/dac tests only
    python3 setup_test_environment.py v3.0.0 --rbp P0 --component m2k # Combined filtering
"""

import os
import sys
import shutil
import argparse
import re
from pathlib import Path


def scan_test_files(source_dir):
    """Find all RST files and extract test metadata."""
    test_files = []
    for root, dirs, files in os.walk(source_dir):
        for file in files:
            if file.endswith('.rst'):
                file_path = os.path.join(root, file)
                relative_path = os.path.relpath(file_path, source_dir)
                component = extract_component_from_path(relative_path)
                tests = parse_rst_tests(file_path)

                test_files.append({
                    'file_path': file_path,
                    'relative_path': relative_path,
                    'component': component,
                    'tests': tests
                })
    return test_files


def parse_rst_tests(file_path):
    """Extract UID/RBP pairs from RST file."""
    tests = []
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()

        # Find UID/RBP patterns (reuse logic from generateTestsTable.py)
        uid_pattern = r'\*\*UID:\*\*\s+([A-Z_][A-Z0-9_.]*)'
        uid_matches = re.findall(uid_pattern, content)

        for uid in uid_matches:
            uid_pos = content.find(f"**UID:** {uid}")
            if uid_pos == -1:
                continue

            # Search for RBP in the next 500 characters after UID
            search_area = content[uid_pos:uid_pos + 500]
            rbp_pattern = r'\*\*RBP:\*\*\s+(P[0-3])'
            rbp_match = re.search(rbp_pattern, search_area)

            rbp = rbp_match.group(1) if rbp_match else 'MISSING'
            tests.append({'uid': uid, 'rbp': rbp})
    except Exception as e:
        print(f"Warning: Error parsing {file_path}: {e}")

    return tests


def extract_component_from_path(relative_path):
    """Map file path to component name."""
    parts = relative_path.split('/')

    if parts[0] == 'plugins' and len(parts) >= 2:
        return parts[1]  # plugins/adc/... -> 'adc'
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


def filter_by_component(test_files, component_filter):
    """Simple file-level filtering by component."""
    if not component_filter:
        return test_files

    filtered = []
    for tf in test_files:
        # Include files from specified components
        if tf['component'] in component_filter:
            filtered.append(tf)
        # Include only top-level index files, not component-specific ones
        elif 'index.rst' in tf['relative_path']:
            path_parts = tf['relative_path'].split('/')
            # Include plugins/index.rst, general/index.rst but not plugins/m2k/index.rst
            if len(path_parts) == 2 and path_parts[1] == 'index.rst':
                filtered.append(tf)

    return filtered


def parse_rst_structure(content):
    """Parse RST into header section + individual test sections."""

    # Find test boundaries using pattern: "Test N - " followed by underline
    test_pattern = r'^(Test \d+[^\n]*\n[\^=-]+\n)'

    # Split content at test boundaries
    parts = re.split(test_pattern, content, flags=re.MULTILINE)

    if len(parts) == 1:
        # No tests found
        return {'header': content, 'tests': []}

    header = parts[0]  # Everything before first test
    tests = []

    # Process test sections (parts[1], parts[2], parts[3], parts[4], ...)
    # parts[1] = "Test 1:" heading, parts[2] = test content
    # parts[3] = "Test 2:" heading, parts[4] = test content, etc.
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
    uid_pattern = r'\*\*UID:\*\*\s+([A-Z_][A-Z0-9_.]*)'
    match = re.search(uid_pattern, content)
    return match.group(1) if match else 'MISSING'


def extract_rbp_from_content(content):
    """Extract RBP from test content block."""
    rbp_pattern = r'\*\*RBP:\*\*\s+(P[0-3])'
    match = re.search(rbp_pattern, content)
    return match.group(1) if match else 'MISSING'


def rebuild_rst_file(header, filtered_tests):
    """Reconstruct RST file with only filtered tests."""
    result = header

    for test in filtered_tests:
        result += test['heading']  # "Test N:" heading with underline
        result += test['content']  # Test content

    return result


def filter_rst_content(file_path, rbp_filter):
    """Parse RST file and keep only tests matching RBP filter."""

    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Split into header + test sections
    structure = parse_rst_structure(content)

    # Filter tests by RBP
    filtered_tests = [test for test in structure['tests']
                     if test['rbp'] in rbp_filter]

    # Reconstruct file
    return rebuild_rst_file(structure['header'], filtered_tests)


def get_copied_components(filtered_files):
    """Track which components were actually copied."""
    copied_components = set()
    for tf in filtered_files:
        if tf['component'] not in ['other', 'general']:
            copied_components.add(tf['component'])
    return copied_components


def extract_component_from_toctree_entry(entry):
    """Extract component name from toctree entry."""
    # "   adc/index" → "adc"
    # "   core/hp_tests" → "core"
    entry = entry.strip()
    if '/' in entry:
        return entry.split('/')[0]
    return entry


def filter_index_content(file_path, copied_components):
    """Filter toctree entries in index files based on copied components."""

    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Find toctree section
    toctree_pattern = r'(\.\. toctree::\n(?:\s+:[^\n]+\n)*\s*\n)(.*?)(\n\n|\.\.|$)'
    match = re.search(toctree_pattern, content, re.DOTALL)

    if not match:
        return content  # No toctree found

    header = match.group(1)
    entries = match.group(2)
    footer = content[match.end():]

    # Filter toctree entries
    filtered_entries = []
    for line in entries.split('\n'):
        if line.strip():
            # Extract component from entry like "adc/index" → "adc"
            component = extract_component_from_toctree_entry(line)
            if component in copied_components:
                filtered_entries.append(line)

    # Reconstruct content
    before_toctree = content[:match.start()]
    new_entries = '\n'.join(filtered_entries)

    return f"{before_toctree}{header}{new_entries}\n{footer}"


def copy_files_unchanged(test_files, dest_dir, component_filter=None):
    """Copy files without content modification."""
    copied_count = 0
    copied_components = get_copied_components(test_files) if component_filter else None

    for test_file in test_files:
        src_path = test_file['file_path']
        dest_path = os.path.join(dest_dir, test_file['relative_path'])

        # Create destination directory
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

        # Filter index files when component filtering is active
        if component_filter and 'index.rst' in test_file['relative_path']:
            filtered_content = filter_index_content(src_path, copied_components)
            with open(dest_path, 'w', encoding='utf-8') as f:
                f.write(filtered_content)
        else:
            # Copy file unchanged
            shutil.copy2(src_path, dest_path)

        copied_count += 1

    return copied_count


def filter_by_rbp_content(test_files, rbp_filter, dest_dir, component_filter=None):
    """Content-level filtering: modify files to keep only specified RBP tests."""
    if not rbp_filter:
        # No RBP filter - copy files as-is
        return copy_files_unchanged(test_files, dest_dir, component_filter)

    copied_count = 0
    copied_components = get_copied_components(test_files) if component_filter else None

    # RBP filter active - parse and modify content
    for test_file in test_files:
        dest_path = os.path.join(dest_dir, test_file['relative_path'])

        # Create destination directory
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

        if test_file['tests']:  # File contains tests
            filtered_content = filter_rst_content(test_file['file_path'], rbp_filter)
            with open(dest_path, 'w', encoding='utf-8') as f:
                f.write(filtered_content)
        elif component_filter and 'index.rst' in test_file['relative_path']:
            # Filter index files when component filtering is active
            filtered_content = filter_index_content(test_file['file_path'], copied_components)
            with open(dest_path, 'w', encoding='utf-8') as f:
                f.write(filtered_content)
        else:  # File has no tests (index, docs, etc.)
            shutil.copy2(test_file['file_path'], dest_path)

        copied_count += 1

    return copied_count


def main():
    parser = argparse.ArgumentParser(
        description='Setup test environment for manual testing',
        epilog='Examples:\n'
               '  %(prog)s v3.0.0\n'
               '  %(prog)s v3.0.0 --rbp P0 P1\n'
               '  %(prog)s v3.0.0 --component adc m2k\n'
               '  %(prog)s v3.0.0 --rbp P0 --component m2k',
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument('version', help='Version name (e.g., v3.0.0)')
    parser.add_argument('--rbp', nargs='+', choices=['P0', 'P1', 'P2', 'P3'],
                       help='Filter by RBP priority levels')
    parser.add_argument('--component', nargs='+',
                       help='Filter by component names (adc, m2k, core, etc.)')

    args = parser.parse_args()

    # Setup paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    scopy_root = os.path.dirname(os.path.dirname(script_dir))  # tools/testing -> root
    source_dir = os.path.join(scopy_root, 'docs', 'tests')
    testing_results_base = os.path.join(scopy_root, 'testing_results')
    dest_dir = os.path.join(testing_results_base, f'testing_results_{args.version}')

    # Validate source directory
    if not os.path.exists(source_dir):
        print(f"Error: Source directory not found: {source_dir}")
        sys.exit(1)

    # Create base testing_results directory if it doesn't exist
    if not os.path.exists(testing_results_base):
        os.makedirs(testing_results_base)

    # Create destination directory
    if os.path.exists(dest_dir):
        response = input(f"Directory {dest_dir} exists. Overwrite? (y/N): ")
        if response.lower() != 'y':
            print("Aborted.")
            sys.exit(1)
        shutil.rmtree(dest_dir)

    os.makedirs(dest_dir)

    # Scan all files
    print(f"Scanning test files in {source_dir}...")
    all_files = scan_test_files(source_dir)
    print(f"Found {len(all_files)} files total")

    # Stage 1: Component filtering (file-level)
    if args.component:
        print(f"Filtering by components: {args.component}")
        component_filtered = filter_by_component(all_files, args.component)
    else:
        component_filtered = all_files

    print(f"After component filter: {len(component_filtered)} files")

    # Stage 2: RBP filtering (content-level) + copying
    if args.rbp:
        print(f"Filtering content by RBP: {args.rbp}")
        copied_count = filter_by_rbp_content(component_filtered, args.rbp, dest_dir, args.component)
    else:
        print("No RBP filter - copying files unchanged")
        copied_count = copy_files_unchanged(component_filtered, dest_dir, args.component)

    print(f"\n✓ Setup complete! {copied_count} files copied to {dest_dir}")


if __name__ == "__main__":
    main()