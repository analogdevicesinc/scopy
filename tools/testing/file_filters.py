#!/usr/bin/env python3
"""
File Filters Module
Handles file and content filtering logic for test environment setup.

This module provides functionality for:
- Component-based file filtering
- RBP-based content filtering
- New test detection and filtering
- Helper functions for filtering decisions
"""

from rst_parser import parse_rst_structure, rebuild_rst_file


def _should_include_file(test_file, component_filter, new_uids):
    """Determine if file should be included based on filters."""
    # Skip all index files - they're not needed for CSV workflow
    if 'index.rst' in test_file['relative_path']:
        return False

    # Special handling: 'general' meta-component includes entire general/ directory
    # This includes both 'core' subdirectory and 'general' files
    if 'general' in component_filter:
        if test_file['relative_path'].startswith('general/'):
            return True

    # Include files from specified components
    if test_file['component'] in component_filter:
        return True

    # Include files containing new tests (bypass component filter)
    if new_uids and any(test['uid'] in new_uids for test in test_file['tests']):
        return True

    return False


def filter_by_component(test_files, component_filter, new_uids=None):
    """Simple file-level filtering by component, with new-since support."""
    if not component_filter:
        return test_files

    return [tf for tf in test_files
            if _should_include_file(tf, component_filter, new_uids)]


def _should_include_test(test, rbp_filter, new_uids):
    """Determine if test should be included based on filters."""
    # Include if it's a new test (always)
    if new_uids and test['uid'] in new_uids:
        return True

    # Include if matches RBP filter
    if rbp_filter and test['rbp'] in rbp_filter:
        return True

    # Include if no filters specified
    if not rbp_filter and not new_uids:
        return True

    return False


def _extract_all_uids(test_files):
    """Extract all UIDs from test files efficiently."""
    return {test['uid'] for file_info in test_files
            for test in file_info['tests']
            if test['uid'] != 'MISSING'}


def filter_rst_content(file_path, rbp_filter, new_uids=None):
    """Parse RST file and keep only tests matching RBP filter or new tests."""

    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    # Split into header + test sections
    structure = parse_rst_structure(content)

    # Filter tests by RBP or new-since
    filtered_tests = [test for test in structure['tests']
                     if _should_include_test(test, rbp_filter, new_uids)]

    # Reconstruct file
    return rebuild_rst_file(structure['header'], filtered_tests)