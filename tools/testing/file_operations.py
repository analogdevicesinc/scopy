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
from file_filters import filter_rst_content


def copy_files_unchanged(test_files, dest_dir, component_filter=None):
    """Copy files without content modification."""
    copied_count = 0

    for test_file in test_files:
        src_path = test_file['file_path']
        dest_path = os.path.join(dest_dir, test_file['relative_path'])

        # Create destination directory
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

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

    # Filter active - parse and modify content
    for test_file in test_files:
        dest_path = os.path.join(dest_dir, test_file['relative_path'])

        # Create destination directory
        os.makedirs(os.path.dirname(dest_path), exist_ok=True)

        if test_file['tests']:  # File contains tests
            filtered_content = filter_rst_content(test_file['file_path'], rbp_filter, new_uids)
            with open(dest_path, 'w', encoding='utf-8') as f:
                f.write(filtered_content)
        else:  # File has no tests (docs, etc.)
            shutil.copy2(test_file['file_path'], dest_path)

        copied_count += 1

    return copied_count