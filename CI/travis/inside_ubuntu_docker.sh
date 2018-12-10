#!/bin/sh -xe

LIBNAME="$1"

cd /$LIBNAME

apt-get -qq update
apt-get -y install sudo
apt-get -y install software-properties-common git wget
pwd
ls
/$LIBNAME/CI/travis/before_install_linux.sh default bionic "$LIBNAME"

/$LIBNAME/CI/travis/make.sh "$LIBNAME" default
. /${LIBNAME}/CI/travis/lib.sh
echo "$(get_ldist)" > /${LIBNAME}/build/.LDIST
