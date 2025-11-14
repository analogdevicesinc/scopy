#!/usr/bin/env python3
"""
Git Operations Module
Handles git repository operations and new test detection.

This module provides functionality for:
- Git repository validation and operations
- Extracting test files from specific git versions
- Detecting new tests between versions
"""

import os
import subprocess
import tempfile
import shutil
from rst_parser import scan_test_files


def get_git_root():
    """Get the git repository root directory."""
    try:
        result = subprocess.run(['git', 'rev-parse', '--show-toplevel'],
                              capture_output=True, check=True, text=True)
        return result.stdout.strip()
    except subprocess.CalledProcessError:
        return None


def validate_git_repository():
    """Ensure we're in a git repository."""
    return get_git_root() is not None


def git_tag_exists(tag_name):
    """Check if a git tag exists."""
    git_root = get_git_root()
    if not git_root:
        return False

    try:
        result = subprocess.run(['git', 'tag', '-l', tag_name],
                              cwd=git_root, capture_output=True, check=True, text=True)
        return tag_name in result.stdout.strip().split('\n')
    except subprocess.CalledProcessError:
        return False


def detect_new_tests(baseline_version):
    """Detect new test UIDs since baseline version."""
    git_root = get_git_root()
    if not git_root:
        raise RuntimeError("Not in a git repository")

    temp_dir = tempfile.mkdtemp()

    try:
        # Extract docs/tests directory at baseline version from git root
        cmd = ['git', 'archive', baseline_version, 'docs/tests']
        with subprocess.Popen(cmd, cwd=git_root, stdout=subprocess.PIPE) as git_proc:
            subprocess.run(['tar', '-x', '-C', temp_dir],
                         stdin=git_proc.stdout, check=True)

        # Scan extracted files for UIDs
        baseline_tests_dir = os.path.join(temp_dir, 'docs', 'tests')
        if not os.path.exists(baseline_tests_dir):
            raise FileNotFoundError(f"docs/tests not found in {baseline_version}")

        baseline_files = scan_test_files(baseline_tests_dir)

        baseline_uids = set()
        for file_info in baseline_files:
            for test in file_info['tests']:
                if test['uid'] != 'MISSING':
                    baseline_uids.add(test['uid'])

        print(f"Baseline version {baseline_version}: {len(baseline_uids)} tests")
        return baseline_uids

    except Exception as e:
        print(f"Error extracting baseline UIDs: {e}")
        raise
    finally:
        shutil.rmtree(temp_dir)