#!/bin/bash

# Print a hash of the upstream commits that install_macos_deps.sh clones.
# Used in the CI cache key so that moving refs (master, main, ...) invalidate the dependencies cache.
# Usage: ./deps_cache_key.sh

set -eo pipefail
REPO_SRC=$(git rev-parse --show-toplevel)
source $REPO_SRC/ci/macOS/macos_config.sh

# Same repositories and refs as clone() in install_macos_deps.sh
DEPS="https://github.com/sigrokproject/libserialport $LIBSERIALPORT_BRANCH
https://github.com/cseci/qwt.git $QWT_BRANCH
https://github.com/KDAB/KDDockWidgets.git $KDDOCK_BRANCH
https://github.com/KDE/extra-cmake-modules.git $ECM_BRANCH
https://github.com/KDE/karchive.git $KARCHIVE_BRANCH
https://github.com/analogdevicesinc/genalyzer.git $GENALYZER_BRANCH
https://github.com/qcoro/qcoro.git $QCORO_BRANCH"
if [ "$SCOPY_LIBIIO_VERSION" == "0" ]; then
	DEPS="$DEPS
https://github.com/analogdevicesinc/libiio.git $LIBIIO_V0_VERSION
https://github.com/analogdevicesinc/libad9361-iio.git $LIBAD9361_BRANCH
https://github.com/analogdevicesinc/libad9166-iio.git $LIBAD9166_BRANCH
https://github.com/analogdevicesinc/libm2k.git $LIBM2K_BRANCH
https://github.com/analogdevicesinc/libtinyiiod.git $LIBTINYIIOD_BRANCH"
else
	DEPS="$DEPS
https://github.com/analogdevicesinc/libiio.git $LIBIIO_VERSION"
fi

while read -r url ref; do
	sha=$(git ls-remote $url refs/heads/$ref refs/tags/$ref)
	if [ -z "$sha" ]; then
		echo "ERROR: cannot resolve $ref in $url" >&2
		exit 1
	fi
	echo "$sha"
done <<< "$DEPS" | shasum -a 256 | cut -d ' ' -f 1
