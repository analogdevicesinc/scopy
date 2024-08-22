#!/bin/bash -xe

# to fix the error "fatal: transport 'file' not allowed"
git config --global protocol.file.allow always

mkdir -p /home/runner/flatpak_tools
pushd /home/runner/flatpak_tools
cp /home/runner/config/* /home/runner/flatpak_tools
git clone https://github.com/flathub/shared-modules.git
make preprocess

flatpak-builder --verbose --keep-build-dirs --ccache --user --disable-rofiles-fuse --force-clean --arch=x86_64 --stop-at=scopy build org.adi.Scopy.json
popd
