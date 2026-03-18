# Scopy Qt6 Build & Runtime Dependencies

## Qt Version
- **Qt 6.7.0** installed via [aqtinstall](https://github.com/miurahr/aqtinstall)
- Additional Qt modules required: `qt3d`, `qtscxml` (for StateMachine)

```bash
pip3 install aqtinstall
python3 -m aqt install-qt --outputdir /opt/Qt linux desktop 6.7.0 -m qt3d qtscxml
```

## System Packages (Ubuntu 24.04)

### Build dependencies (inherited from Qt5 build)
```
python3-full python3-pip python3-numpy python3-packaging python3-mako
g++ build-essential cmake curl autogen autoconf autoconf-archive pkg-config flex bison swig
subversion mesa-common-dev graphviz xserver-xorg gettext texinfo mm-common doxygen
libboost-all-dev libfftw3-dev liblog4cpp5v5 liblog4cpp5-dev
libxcb-xinerama0 libgmp3-dev libzip-dev libglib2.0-dev libglibmm-2.4-dev libsigc++-2.0-dev
libclang1 libmatio-dev liborc-0.4-dev libgl1-mesa-dev libavahi-client-dev libavahi-common-dev
libusb-1.0-0 libusb-1.0-0-dev libsndfile1-dev
libxkbcommon-x11-0 libncurses-dev libtool libaio-dev libzmq3-dev libxml2-dev
```

### Qt6-specific build dependencies
```
libglu1-mesa-dev libxkbcommon-dev libvulkan-dev
libzstd-dev libbz2-dev liblzma-dev
```

### Qt6 runtime dependencies (xcb platform plugin)
```
libxcb-cursor0 libxcb-icccm4 libxcb-keysyms1 libxcb-shape0
```

## Library Dependencies (built from source)

| Library | Branch/Version | Notes |
|---------|---------------|-------|
| libserialport | master | |
| libiio | v0.26 | |
| libad9361 | main | |
| libm2k | main | |
| spdlog | v1.x | |
| volk | main | |
| gnuradio | scopy2-maint-3.10 | |
| gr-scopy | 3.10 | |
| gr-m2k | main | |
| qwt | qwt-multiaxes-updated | Built with qmake6 |
| libsigrokdecode | master | |
| libtinyiiod | master | |
| iio-emu | main | |
| KDDockWidgets | 2.2 | Requires `-DKDDockWidgets_QT6=ON` |
| extra-cmake-modules | v6.7.0 | Version-matched to Qt |
| KArchive | v6.7.0 | Version-matched to Qt |
| genalyzer | main | |

## Docker Development Environment

Build the Docker image:
```bash
cd ci/ubuntu
docker build -f docker_ubuntu/Dockerfile.qt6 -t scopy-qt6-dev .
```

Run with X11 display forwarding:
```bash
xhost +local:docker
docker run -it --rm -v /path/to/scopy:/home/runner/scopy -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY --net=host scopy-qt6-dev bash
```

Build Scopy inside the container:
```bash
git config --global --add safe.directory /home/runner/scopy
cd /home/runner/scopy
cmake -B build-qt6 -DCMAKE_PREFIX_PATH=/opt/Qt/6.7.0/gcc_64 -DENABLE_ALL_PACKAGES=ON
cmake --build build-qt6 -j$(nproc)
./build-qt6/scopy
```

## CMake Configuration

```bash
cmake -B build \
  -DCMAKE_PREFIX_PATH=/opt/Qt/6.7.0/gcc_64 \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DENABLE_ALL_PACKAGES=ON
```
