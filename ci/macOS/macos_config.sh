#!/bin/bash

STAGING_AREA=$PWD/staging
STAGING_AREA_DEPS=$STAGING_AREA/dependencies
REPO_SRC=$(git rev-parse --show-toplevel)
BUILDDIR=$REPO_SRC/build
JOBS=-j8
QT_FORMULAE=qt@5

# In the Boost 1.89.0 release,
# the Boost.System library, needed by GNU Radio, has been removed
# Issue explained here: https://github.com/powerdns/pdns/issues/15972
# Workaround is to use Boost 1.85 for now
BOOST_FORMULAE=boost@1.85

QT_PATH="$(brew --prefix ${QT_FORMULAE})/bin"
BOOST_PATH="$(brew --prefix ${BOOST_FORMULAE})/bin"
export PATH="${QT_PATH}:${BOOST_PATH}:$PATH"
export LD_LIBRARY_PATH="$LD_LIBRARY_PATH;$STAGING_AREA_DEPS;$STAGING_AREA_DEPS/lib"


LIBSERIALPORT_BRANCH=master
LIBIIO_VERSION=v0.26
LIBAD9361_BRANCH=main
LIBM2K_BRANCH=main
GNURADIO_BRANCH=scopy2-maint-3.10
GRSCOPY_BRANCH=3.10
GRM2K_BRANCH=main
QWT_BRANCH=qwt-multiaxes-updated
LIBSIGROKDECODE_BRANCH=master
LIBTINYIIOD_BRANCH=master
KDDOCK_BRANCH=2.1
ECM_BRANCH=kf5
KARCHIVE_BRANCH=kf5