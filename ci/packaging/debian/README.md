# Packaging Directory

## Folder overview

This directory contains scripts and configuration for packaging Scopy as a debian package.

Currently there is support only for ARM 32/64 architecture and in order to solve problem of locally built dependencies, the deb package actually installs the Scopy.AppImage along with the desktop configuration files.

The script `scopy-deb-build.sh` is used in the *.github/workflows/appimage-armhf.yml* and *.github/workflows/appimage-arm64.yml* workflows.
