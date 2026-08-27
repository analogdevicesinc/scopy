#!/bin/bash -xe

# Flatpak Build Process Script
# ===========================
# Purpose: Build Scopy as a Flatpak package
# Usage: ./flatpak_build_process.sh
#
# This script:
# 1. Generates the Flatpak manifest (JSON)
# 2. Modifies it for CI builds
# 3. Builds the Flatpak package

# Get repository root directory
SCOPY_DIR=$(git rev-parse --show-toplevel 2>/dev/null ) || \
SCOPY_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && cd ../../ && pwd )

# Flatpak manifest file (generated from template)
SCOPY_JSON=$SCOPY_DIR/ci/flatpak/org.adi.Scopy.json

pushd $SCOPY_DIR/ci/flatpak

git submodule update --init

# Generate Flatpak manifest from template
# The template (.json.c) contains C preprocessor directives
make preprocess

export EN_PREPROCESS=false

if [ "$CI_SCRIPT" == "ON" ];
	then
		SOURCE_DIR=$GITHUB_WORKSPACE
		# Use pre-built dependencies from Docker image
		# .flatpak-builder: Flatpak's build cache
		# build: Compiled dependencies
		# Seed the cache only when the workspace does not already have one. A plain "cp -r"
		# into a populated .flatpak-builder fails: git writes its pack files mode 444, so cp
		# cannot overwrite them ("cannot create regular file ...: Permission denied"). CI is
		# unaffected either way - actions/checkout gives a virgin workspace, so the copy always
		# runs there - but a second local run in the same tree aborts here under -e. Reusing the
		# existing cache is also the point of the cache: the rerun resumes at the first module
		# whose definition changed. To force a refresh from the image, delete
		# $SOURCE_DIR/ci/flatpak/.flatpak-builder first.
		if [ -d $SOURCE_DIR/ci/flatpak/.flatpak-builder ]; then
			echo "-- Reusing existing flatpak-builder cache in the workspace"
		else
			cp -r /home/runner/flatpak_tools/.flatpak-builder $SOURCE_DIR/ci/flatpak
		fi
		# "make preprocess" above removes build/ every run, so this copy is already idempotent.
		cp -r /home/runner/flatpak_tools/build $SOURCE_DIR/ci/flatpak
	else
		SOURCE_DIR=$SCOPY_DIR
fi

# The manifest normally fetches Scopy from git
# For CI, we need to use the local source directory

# Find Scopy module (it's the last one in the modules array)
cnt=$( echo $(jq '.modules | length' $SCOPY_JSON) )
cnt=$(($cnt-1))

# Replace git source with local directory source
cat $SCOPY_JSON | jq --tab '.modules['$cnt'].sources[0].type = "dir"' > tmp.json
cp tmp.json $SCOPY_JSON
# Set path to current source directory
cat $SCOPY_JSON | jq --tab '.modules['$cnt'].sources[0].path = "'$SOURCE_DIR'"' > tmp.json
cp tmp.json $SCOPY_JSON
cat $SCOPY_JSON | jq --tab 'del(.modules['$cnt'].sources[0].url)' > tmp.json
cp tmp.json $SCOPY_JSON
cat $SCOPY_JSON | jq --tab 'del(.modules['$cnt'].sources[0].branch)' > tmp.json
cp tmp.json $SCOPY_JSON
# Exclude build artefacts and other-platform staging dirs from the copy. Two reasons:
# ci/arm/staging/sysroot/dev/ holds device nodes, which flatpak-builder cannot copy
# ("Error: module scopy: Can't copy special file"); and ci/flatpak/.flatpak-builder is
# flatpak-builder's own cache, copied in above, so without this the source copy drags in
# the whole prebuilt dependency tree. skip paths are relative to the source dir and prune
# entire subtrees, and are literal relative paths - g_file_resolve_relative_path(), so no globs,
# which is why the Kuiper image is named in full. Every entry is gitignored, so on a clean CI
# checkout only the ci/flatpak ones exist - and CI does populate those.
# ci/arm/docker/tarballs (1.85 GB) and the Kuiper .img (4.8 GB) are armhf image-build inputs; a
# local flatpak build was copying both on every run, since dir sources are never cached.
# Not skipping .git: ScopyAbout.cmake runs git rev-parse/config/log at configure time.
cat $SCOPY_JSON | jq --tab '.modules['$cnt'].sources[0].skip = ["ci/arm/staging", "ci/arm/docker/tarballs", "ci/arm/image_ADI-Kuiper-Linux-armhf.img", "ci/flatpak/.flatpak-builder", "ci/flatpak/build", "ci/flatpak/repo", ".worktrees", "build"]' > tmp.json
cp tmp.json $SCOPY_JSON
rm tmp.json

# Extract version info from all git modules for the build status file
jq '.modules[] | select(type == "object" and .sources[]?.type == "git") | "\(.name): \(.sources[] | select(.type == "git") | .branch // .tag // .commit // "no branch, tag, or commit")"' ./org.adi.Scopy.json >> build-status
cp build-status $SOURCE_DIR/build-status

# Insert env vars in the sandboxed flatpak build
# $SOURCE_DIR/ci/general/gen_ci_envs.sh > $SOURCE_DIR/ci/general/gh-actions.envs
# CI_ENVS=$(jq -R -n -c '[inputs|split("=")|{(.[0]):.[1]}] | add' $SOURCE_DIR/ci/general/gh-actions.envs)
# echo "CI_ENVS= $CI_ENVS"
# cat $SCOPY_JSON | jq --tab '."build-options".env += ('$CI_ENVS')' > tmp.json
# cp tmp.json $SCOPY_JSON

# Build the Flatpak
# This uses flatpak-builder with the generated manifest
make

# Copy the Scopy.flatpak file in $SOURCE_DIR (which is the external location, mount when docker starts)
[ -z $CI_SCRIPT ] || cp Scopy.flatpak $SOURCE_DIR/
popd
