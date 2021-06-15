#!/bin/sh -xe

cd /home
apt-get -y install flatpak-builder git wget cmake

git clone https://github.com/analogdevicesinc/scopy-flatpak -b test
cd scopy-flatpak
make -j4

ls

