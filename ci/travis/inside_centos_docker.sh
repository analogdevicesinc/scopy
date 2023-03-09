#!/bin/sh -xe

LIBNAME="$1"

cd /$LIBNAME

/$LIBNAME/CI/travis/before_install_linux.sh centos centos7 "$LIBNAME"

export TRAVIS="false"

echo $PATH
echo $PKG_CONFIG_PATH

ls /usr/local/lib/pkgconfig 
ls /usr/local/lib64/pkgconfig


/$LIBNAME/CI/travis/make_linux.sh "$LIBNAME" centos
. /${LIBNAME}/CI/travis/lib.sh
echo "$(get_ldist)" > /${LIBNAME}/build/.LDIST
