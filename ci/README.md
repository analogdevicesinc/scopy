
This file contains dependency information for different platforms

**Windows (x86_64)** - 
https://github.com/analogdevicesinc/scopy-mingw-build-deps 
builds the docker image - `docker pull analogdevices/scopy-build:mingw64`
Github Actions workflow: 
https://github.com/analogdevicesinc/scopy/blob/master/.github/workflows/mingwbuild.yml

**Linux (flatpak - x86_64)**
https://github.com/analogdevicesinc/scopy-flatpak
docker pull analogdevices/scopy-build:flatpak

    ARCH=x86_64 make
   
Github Actions workflow - https://github.com/analogdevicesinc/scopy/blob/master/.github/workflows/linuxflatpakbuild.yml

**Linux (flatpak - arm)**
https://github.com/analogdevicesinc/scopy-flatpak
Run locally on arm machine (raspberry pi)

    ARCH=arm make
Not build in CI

**Linux (ubuntu - x86_64) - development**
Built on appveyor - https://github.com/analogdevicesinc/scopy/blob/master/appveyor.yml
**macOS (x86_64)**
Built on appveyor - https://github.com/analogdevicesinc/scopy/blob/master/appveyor.yml

**Android (aarch64)**
https://github.com/analogdevicesinc/scopy-android-deps  - `docker pull analogdevices/scopy-build:android`
Github Actions workflow - https://github.com/analogdevicesinc/scopy/blob/master/.github/workflows/androidbuild.yml

Dependency versions (links used in source builds)

 - Qt: 5.15.12 
 - Qwt - https://github.com/cseci/qwt - qwt-multiaxes
 - libiio - https://github.com/analogdevicesinc/libiio - https://github.com/analogdevicesinc/libiio/tree/cad83146837971acdac28beaeb8156b9da33ba6b - v0.24
	 - libxml2 - https://github.com/GNOME/libxml2
		 - iconv (only on Android from src)
		 - libffi (only on Android from src)
		 - libgettext (only on Android from src)
	 - libusb - https://downloads.sourceforge.net/project/libusb/libusb-1.0/libusb-1.0.24/libusb-1.0.24.tar.bz2
		 - Android specific libusb: https://github.com/xloem/libusb/tree/d1856aa8c246f9e56cf00a0765462b67fc5a4871
 - libm2k- https://github.com/analogdevicesinc/libm2k - master
	 - glog - https://github.com/google/glog
 - boost - 
 - gnuradio - https://github.com/analogdevicesinc/gnuradio - scopy / scopy-android-2(for Android)
	 - volk
	 - log4cpp - https://github.com/cseci/log4cpp
	 - fftw3
	 - libgmp
 - gr-iio https://github.com/analogdevicesinc/gr-iio - upgrade3.8
	 - libad9361 - https://github.com/analogdevicesinc/ad9361
 - gr-m2k - https://github.com/analogdevicesinc/gr-m2k - master
 - gr-scopy - https://github.com/analogdevicesinc/gr-scopy - master
 - libsigrokdecode - https://github.com/sigrokproject/libsigrokdecode - master
	 - glib
	 - glibmm
	 - sigcpp
	 - python
 - libtinyiiod - https://github.com/analogdevicesinc/libtinyiiod - master

How to install Qt from qt.io : https://github.com/analogdevicesinc/scopy-android-deps/blob/master/docker/Dockerfile#L43-L49


| Dependency | Windows | Linux Flatpak | Linux Ubuntu(development) | Linux ARM | macOS | Android |
| --- | --- | --- | --- | --- | --- | --- |
| Qt | pacman  | org.kde.Sdk (v5.15)| Qt.io |org.kde.Sdk (v5.14) | brew | Qt.io |
| qwt | src | src | src | src | src | src |
| libxml2 | pacman | src | apt | src | brew | src |
| libusb | pacman | src | apt | src | brew | src -  android branch/commit |
| libiio | src | src | src | src | src | src|
| glog | src | src | src | src | brew | src |
| libm2k | src | src | src | src | src | src |
| volk | src | with GR | with GR | src | with GR | src |
| fftw3 | pacman| src | apt | src | brew| src|
| libgmp | pacman | src | apt | src | brew | src |
| boost | 1.75 | 1.72 | apt/src | 1.72 | brew | Boost-for-Android |
| gnuradio | src | src | apt/src | src | src | src |
| gr-iio | src | src | src | src | src | src |
| gr-m2k | src | src | src | src | src | src |
| gr-scopy | src | src | src | src | src | src |
| glib | pacman | src | apt | src | brew | src |
| glibmm | pacman | src | apt | src | src | src |
| sigcpp | pacman | src | apt | src | src | src |
| python | pacman | src | apt | src | brew | src |
| libsigrokdecode | src | src | src | src | src | src |
| libtinyiiod | src | src | src | src | src | src|
