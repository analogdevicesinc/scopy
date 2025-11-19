#!/usr/bin/env python3
"""
Test Environment Setup Script
Creates filtered copies of test documentation for manual testing execution.

Usage:
    python3 setup_test_environment.py v3.0.0                          # Copy all tests
    python3 setup_test_environment.py v3.0.0 --rbp P0 P1              # Copy P0/P1 tests only
    python3 setup_test_environment.py v3.0.0 --component adc dac      # Copy adc/dac tests only
    python3 setup_test_environment.py v3.0.0 --rbp P0 --component m2k # Combined filtering
    python3 setup_test_environment.py v3.0.0 --new-since v2.1.0       # New tests since v2.1.0
    python3 setup_test_environment.py v3.0.0 --rbp P0 --new-since v2.1.0 # P0 + new tests
"""

import os
import sys
import shutil
import argparse

# Import from our new modules
from git_operations import validate_git_repository, git_tag_exists, detect_new_tests
from rst_parser import scan_test_files
from file_filters import filter_by_component, _extract_all_uids
from file_operations import copy_files_unchanged, filter_by_rbp_content


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
    parser.add_argument('--new-since',
                       help='Include tests added since specified version (e.g., v2.1.0)')

    args = parser.parse_args()

    # Validate git requirements for --new-since
    if args.new_since:
        if not validate_git_repository():
            print("Error: Not in a git repository. --new-since requires git.")
            sys.exit(1)
        if not git_tag_exists(args.new_since):
            print(f"Error: Git tag '{args.new_since}' not found.")
            sys.exit(1)

    # Setup paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    scopy_root = os.path.dirname(os.path.dirname(script_dir))  # tools/testing -> root

    # Validate component names
    if args.component:
        plugins_dir = os.path.join(scopy_root, 'docs', 'tests', 'plugins')
        if os.path.exists(plugins_dir):
            valid_components = [d for d in os.listdir(plugins_dir)
                              if os.path.isdir(os.path.join(plugins_dir, d))]

            for component in args.component:
                if component not in valid_components:
                    print(f"Error: Component '{component}' not found.")
                    sys.exit(1)

    # Continue with paths
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

    # Detect new tests if --new-since specified
    new_uids = None
    if args.new_since:
        print(f"Detecting new tests since {args.new_since}...")
        baseline_uids = detect_new_tests(args.new_since)

        # Get current UIDs
        current_uids = _extract_all_uids(all_files)

        # Calculate new UIDs
        new_uids = current_uids - baseline_uids
        print(f"Current version: {len(current_uids)} tests")
        print(f"New tests detected: {len(new_uids)} tests")

    # Stage 1: Component filtering (file-level) with new-since support
    if args.component:
        print(f"Filtering by components: {args.component}")
        component_filtered = filter_by_component(all_files, args.component, new_uids)
    else:
        component_filtered = all_files

    print(f"After component filter: {len(component_filtered)} files")

    # Stage 2: Content-level filtering + copying
    if args.rbp or args.new_since:
        filter_desc = []
        if args.rbp:
            filter_desc.append(f"RBP: {args.rbp}")
        if args.new_since:
            filter_desc.append(f"new since {args.new_since}")
        print(f"Filtering content by {' + '.join(filter_desc)}")
        copied_count = filter_by_rbp_content(component_filtered, args.rbp, dest_dir, args.component, new_uids)
    else:
        print("No content filters - copying files unchanged")
        copied_count = copy_files_unchanged(component_filtered, dest_dir, args.component)

    print(f"\n✓ Setup complete! {copied_count} files copied to {dest_dir}")

    # Generate CSV template after successful setup
    print("\nGenerating CSV template...")
    try:
        import subprocess
        script_dir = os.path.dirname(os.path.abspath(__file__))
        parse_script = os.path.join(script_dir, 'parseTestResults.py')

        result = subprocess.run([
            'python3', parse_script, args.version
        ], capture_output=True, text=True, check=True)

        print("✓ CSV template generated successfully")

    except subprocess.CalledProcessError as e:
        print(f"Warning: CSV generation failed: {e.stderr.strip()}")
        print("You can manually generate it later with:")
        print(f"  python3 parseTestResults.py {args.version}")
    except Exception as e:
        print(f"Warning: Could not generate CSV template: {e}")
        print("You can manually generate it later with:")
        print(f"  python3 parseTestResults.py {args.version}")


if __name__ == "__main__":
    main()