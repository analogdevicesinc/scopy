#!/usr/bin/env python3
"""
File Operations Module
Handles file copying operations and index file management.

This module provides functionality for:
- Copying test files with or without modification
- Managing index file toctree entries
- Component tracking for copied files
- RST content-level filtering and file operations
"""

import os
import shutil
import re
from file_filters import filter_rst_content


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


def filter_by_rbp_content(test_files, rbp_filter, dest_dir, component_filter=None, new_uids=None):
    """Content-level filtering: modify files to keep only specified RBP tests or new tests."""
    if not rbp_filter and not new_uids:
        # No filters - copy files as-is
        return copy_files_unchanged(test_files, dest_dir, component_filter)

    copied_count = 0
    copied_components = get_copied_components(test_files) if component_filter else None

    # Filter active - parse and modify content
    for test_file in test_files:
        dest_path = os.path.join(dest_dir, test_file['relative_path'])

        # Create destination directory
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

        if test_file['tests']:  # File contains tests
            filtered_content = filter_rst_content(test_file['file_path'], rbp_filter, new_uids)
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