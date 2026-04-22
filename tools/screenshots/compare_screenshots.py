#!/usr/bin/env python3
"""
compare_screenshots.py — Compare two sets of Scopy documentation screenshots.

Compares images using pixel difference percentage to tolerate minor
text changes (e.g. timestamps) while catching real UI differences.

Usage:
    python3 tools/screenshots/compare_screenshots.py \
        --baseline ~/Downloads/screenshots-run1 \
        --current ~/Downloads/screenshots-run2

    # With custom threshold (default: 5% different pixels):
    python3 tools/screenshots/compare_screenshots.py \
        --baseline ~/Downloads/screenshots-run1 \
        --current ~/Downloads/screenshots-run2 \
        --threshold 8
"""

import argparse
import os
import sys

from PIL import Image


def collect_pngs(root):
    """Walk a directory and return a set of relative paths to all .png files."""
    pngs = set()
    for dirpath, _, filenames in os.walk(root):
        for f in filenames:
            if f.lower().endswith(".png"):
                rel = os.path.relpath(os.path.join(dirpath, f), root)
                pngs.add(rel)
    return pngs


def pixel_diff_percent(path_a, path_b):
    """Compare two images and return the percentage of differing pixels."""
    img_a = Image.open(path_a).convert("RGB")
    img_b = Image.open(path_b).convert("RGB")

    if img_a.size != img_b.size:
        return 100.0

    pixels_a = list(img_a.getdata())
    pixels_b = list(img_b.getdata())
    total = len(pixels_a)
    diff = sum(1 for a, b in zip(pixels_a, pixels_b) if a != b)
    return (diff / total) * 100.0


def main():
    parser = argparse.ArgumentParser(
        description="Compare two sets of Scopy documentation screenshots"
    )
    parser.add_argument(
        "--baseline", required=True, help="Path to baseline screenshots"
    )
    parser.add_argument(
        "--current", required=True, help="Path to current screenshots"
    )
    parser.add_argument(
        "--threshold",
        type=float,
        default=5.0,
        help="Max percentage of different pixels to consider images the same "
        "(default: 5.0)",
    )
    args = parser.parse_args()

    baseline_root = os.path.abspath(args.baseline)
    current_root = os.path.abspath(args.current)

    for label, path in [("Baseline", baseline_root), ("Current", current_root)]:
        if not os.path.isdir(path):
            print(f"ERROR: {label} path does not exist: {path}")
            sys.exit(1)

    baseline_files = collect_pngs(baseline_root)
    current_files = collect_pngs(current_root)

    common = sorted(baseline_files & current_files)
    baseline_only = sorted(baseline_files - current_files)
    current_only = sorted(current_files - baseline_files)

    different = []
    for rel in common:
        pct = pixel_diff_percent(
            os.path.join(baseline_root, rel),
            os.path.join(current_root, rel),
        )
        if pct > args.threshold:
            different.append((rel, pct))

    has_issues = bool(different or baseline_only or current_only)

    if not has_issues:
        print(f"All looks the same ({len(common)} images compared)")
        return

    if different:
        print(f"Different images ({len(different)}):")
        for rel, pct in different:
            print(f"  {rel} ({pct:.1f}% pixels differ)")
        print()

    if baseline_only or current_only:
        print(
            f"Images found in one set only ({len(baseline_only) + len(current_only)}):"
        )
        for rel in baseline_only:
            print(f"  [baseline only] {rel}")
        for rel in current_only:
            print(f"  [current only]  {rel}")
        print()

    print(
        f"Summary: {len(common)} compared, {len(different)} different, "
        f"{len(baseline_only)} baseline-only, {len(current_only)} current-only"
    )
    sys.exit(1)


if __name__ == "__main__":
    main()
