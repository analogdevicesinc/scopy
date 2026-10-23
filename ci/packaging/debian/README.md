# Packaging Directory

## Folder overview

This directory contains the Debian packaging definition for Scopy. To solve the
problem of locally built dependencies, the `.deb` actually installs the
`Scopy.AppImage` along with the desktop/polkit configuration files.

## How the `.deb` is built

The `.deb` is produced in CI by the shared
[`deb-generator-action`](https://github.com/analogdevicesinc/shared-actions/tree/main/deb-generator-action),
which runs `debuild` over this `debian/` directory:

- `.github/workflows/appimage-x86_64.yml` calls the composite action directly via `uses:`.
- `.github/workflows/appimage-arm64.yml` and `.github/workflows/appimage-armhf.yml`
  check out `shared-actions` and run its `build-deb.sh` inside a cross-arch container,
  since the composite action only runs on the host.

`build-deb.sh` substitutes `@VERSION@`/`@DATE@` in `changelog` and `@ARCHITECTURE@`
in `control`. The AppImage is pulled into the package by `rules`, which reads its
path from `DEB_APPIMAGE_PATH` (the `DEB_*` prefix is required so the variable
survives `debuild`'s environment sanitization). Currently there is support for
x86_64 and ARM 32/64.
