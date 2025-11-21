#!/usr/bin/env python3
"""
RST Parser Module
Handles parsing of reStructuredText test files and component mapping.

This module provides the core functionality for:
- Extracting test metadata (UID, RBP) from RST files
- Parsing RST structure into header and test sections
- Component mapping from file paths
- RST file reconstruction
"""

import os
import re

# Regex patterns for UID/RBP extraction
UID_PATTERN = r'\*\*UID:\*\*\s+([A-Z_][A-Z0-9_.]*)'
RBP_PATTERN = r'\*\*RBP:\*\*\s+(P[0-3])'


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

        # Find UID patterns using shared constant
        uid_matches = re.findall(UID_PATTERN, content)

        for uid in uid_matches:
            uid_pos = content.find(f"**UID:** {uid}")
            if uid_pos == -1:
                continue

            # Search for RBP in the next 500 characters after UID
            search_area = content[uid_pos:uid_pos + 500]
            rbp = extract_rbp_from_content(search_area)

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


def parse_rst_structure(content):
    """Parse RST into header section + individual test sections."""

    # Find test boundaries using pattern: "Test N " followed by underline
    test_pattern = r'^(Test \d+[^\n]*\n[\^=-]+\n)'

    # Split content at test boundaries
    parts = re.split(test_pattern, content, flags=re.MULTILINE)

    if len(parts) == 1:
        # No tests found
        return {'header': content, 'tests': []}

    header = parts[0]  # Everything before first test
    tests = []

    # Process test sections (parts[1], parts[2], parts[3], parts[4], ...)
    # parts[1] = "Test 1 " heading, parts[2] = test content
    # parts[3] = "Test 2 " heading, parts[4] = test content, etc.
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
    match = re.search(UID_PATTERN, content)
    return match.group(1) if match else 'MISSING'


def extract_rbp_from_content(content):
    """Extract RBP from test content block."""
    match = re.search(RBP_PATTERN, content)
    return match.group(1) if match else 'MISSING'


def rebuild_rst_file(header, filtered_tests):
    """Reconstruct RST file with only filtered tests."""
    result = header

    for test in filtered_tests:
        result += test['heading']  # "Test N:" heading with underline
        result += test['content']  # Test content

    return result