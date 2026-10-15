#!/bin/bash -xe

# to fix the error "fatal: transport 'file' not allowed"
git config --global protocol.file.allow always

mkdir -p /home/runner/flatpak_tools
pushd /home/runner/flatpak_tools
cp /home/runner/config/* /home/runner/flatpak_tools
git clone https://github.com/flathub/shared-modules.git
# One image serves both libiio versions. Each gets its own state dir: the two manifests differ only in the
# modules after qcoro, but a single cache could be garbage collected down to whichever was built last.
for SCOPY_LIBIIO_VERSION in 1 0; do
	[ "$SCOPY_LIBIIO_VERSION" == "0" ] && STATE_DIR=.flatpak-builder-libiio-v0 || STATE_DIR=.flatpak-builder
	make preprocess SCOPY_LIBIIO_VERSION=$SCOPY_LIBIIO_VERSION
	flatpak-builder --verbose --keep-build-dirs --ccache --user --disable-rofiles-fuse --force-clean --state-dir=$STATE_DIR --arch=x86_64 --stop-at=scopy build org.adi.Scopy.json
done
popd
