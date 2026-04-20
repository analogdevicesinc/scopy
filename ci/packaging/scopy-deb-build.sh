#!/bin/bash -e

# Script that generates Scopy .deb package and source files using debhelper
# Usage: ./scopy-deb-build.sh <config> <appimage_path>
#   config:        Configuration file name (scopy32 or scopy64)
#   appimage_path: Path to the already-built AppImage

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
CONFIG=$1
APPIMAGE_PATH=$2

if [ -z "$CONFIG" ] || [ -z "$APPIMAGE_PATH" ]; then
    echo "Usage: $0 <config> <appimage_path>"
    echo "  config:        Configuration file (scopy32 or scopy64)"
    echo "  appimage_path: Path to the already-built Scopy AppImage"
    exit 1
fi

if [ ! -f "$APPIMAGE_PATH" ]; then
    echo "Error: AppImage not found at ${APPIMAGE_PATH}"
    exit 1
fi

# Find the repo root
REPO_ROOT=$(git -C "${SCRIPT_DIR}" rev-parse --show-toplevel)

# Extract SCOPY_VERSION from CMakeLists.txt by resolving its components
PROJECT_VERSION=$(grep -oP 'project\(scopy VERSION \K[0-9.]+' "${REPO_ROOT}/CMakeLists.txt")
RELEASE_PHASE=$(grep -oP 'set\(SCOPY_RELEASE_PHASE \K[^)]+' "${REPO_ROOT}/CMakeLists.txt" || true)
[ "${RELEASE_PHASE}" == "\"\"" ] &&  unset RELEASE_PHASE # if RELEASE_PHASE contains an empty string (only quotes) set it to null
VERSION="${PROJECT_VERSION}${RELEASE_PHASE}"
RELEASE="v${VERSION}"

echo "Scopy version: ${VERSION}"

# Source the config file corresponding to the .deb package
set -a
source "${SCRIPT_DIR}/configs/${CONFIG}.conf"
set +a

export SCOPY=Scopy.AppImage
export SCOPY_ROOT=scopy-root
export APPIMAGE_PATH=$(readlink -f "$APPIMAGE_PATH")

WORK_DIR="${PACKAGE}-${VERSION}"

# Create working directory from local checkout (excluding .git)
echo "Creating working directory from local checkout..."
rsync -a --exclude='.git' --exclude='ci/arm/staging' --exclude='build*' "${REPO_ROOT}/" "${WORK_DIR}/"

# Create .orig.tar.gz for Debian source package
tar czf ${PACKAGE}_${VERSION}.orig.tar.gz ${WORK_DIR}

# Copy debian directory into the working directory
cp -r "${SCRIPT_DIR}/debian" ${WORK_DIR}/

cd ${WORK_DIR}

# Export all variables for envsubst and debian/rules
export VERSION ARCHITECTURE DEPENDS MAINTAINER HOMEPAGE DESCRIPTION LONG_DESCRIPTION SCOPY PACKAGE RELEASE
export DATE=$(date -R)

# Fetch release notes from GitHub
echo "Fetching release notes from GitHub..."
RELEASE_NOTES=$(curl -s https://api.github.com/repos/analogdevicesinc/scopy/releases/tags/${RELEASE} \
  | jq -r '.body // "No release notes available"' \
  | sed '/^[#]*[[:space:]]*Installer Artifacts/,$d' \
  | sed 's/^/  * /')

# If release notes are empty or error occurred, use default
if [ -z "$RELEASE_NOTES" ]; then
    RELEASE_NOTES="  * New upstream release ${VERSION}"
fi

export RELEASE_NOTES

# Substitute templates
envsubst < debian/control.template > debian/control
envsubst < debian/changelog.template > debian/changelog
envsubst < debian/scopy.desktop.template > debian/scopy.desktop
envsubst < debian/org.adi.scopy.policy.template > debian/org.adi.scopy.policy
envsubst '${SCOPY}' < debian/scopy-root.template > debian/scopy-root

# Remove template files
rm debian/*.template

# Make sure debian/rules is executable
chmod +x debian/rules

# Make sure debian/scopy-root is executable
chmod +x debian/scopy-root

# Build package using debhelper (builds both binary and source packages)
echo "Building packages..."
dpkg-buildpackage -us -uc -a ${ARCHITECTURE}

cd ..

echo ""
echo "Build complete! Generated files:"
echo "  - ${PACKAGE}_${VERSION}.orig.tar.gz (upstream source)"
echo "  - ${PACKAGE}_${VERSION}-1.debian.tar.xz (debian files)"
echo "  - ${PACKAGE}_${VERSION}-1.dsc (source descriptor)"
echo "  - ${PACKAGE}_${VERSION}-1_${ARCHITECTURE}.deb (binary package)"
echo "  - ${PACKAGE}_${VERSION}-1_${ARCHITECTURE}.buildinfo"
echo "  - ${PACKAGE}_${VERSION}-1_${ARCHITECTURE}.changes"
