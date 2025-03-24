#!/bin/bash

source ./android_toolchain.sh

clone() {


	popds
}

download_deps() {
#	rm -rf $DEPS_SRC_PATH
#	mkdir -p $DEPS_SRC_PATH
#	pushd $DEPS_SRC_PATH

#	wget https://download.gnome.org/sources/glib/2.58/glib-2.58.3.tar.xz
#	wget http://ftp.acc.umu.se/pub/gnome/sources/glibmm/2.58/glibmm-2.58.1.tar.xz
#	wget http://ftp.acc.umu.se/pub/GNOME/sources/libsigc++/2.10/libsigc++-2.10.0.tar.xz

#	popd
echo "nothing to download"
}



recurse_submodules
#download_deps
#install_jdk
