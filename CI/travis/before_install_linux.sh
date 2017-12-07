#!/bin/sh

sudo add-apt-repository --yes ppa:fkrull/deadsnakes
sudo add-apt-repository --yes ppa:beineri/opt-qt592-trusty
sudo apt-get -qq update
sudo apt-get install -y git cmake libzip-dev libusb-1.0-0-dev autoconf libtool libxml2 libxml2-dev python3.5 python-dev python3.5-dev libfftw3-dev libffi-dev
sudo apt-get install -y libmount-dev libpcre3-dev libglib2.0-dev libsigc++-2.0-dev libglibmm-2.4-dev doxygen libglu1-mesa-dev curl flex bison libmatio2 libmatio-dev
sudo apt-get install -y --force-yes qt59base qt59declarative qt59quickcontrols qt59svg qt59tools

source /opt/qt59/bin/qt59-env.sh && qmllint client/qml/*.qml
cd ${WORKDIR}

sudo mv /usr/bin/python3 /usr/bin/python3-old
sudo ln -s /usr/bin/python3.5 /usr/bin/python3
sudo ln -s /usr/bin/python3.5m /usr/bin/python3m
sudo ln -s /usr/bin/python3.5m-config /usr/bin/python3m-config
sudo sed -i "s/4/5/g" /usr/lib/x86_64-linux-gnu/pkgconfig/python3.pc

mkdir -p deps
cd deps
WORKDIR=$PWD

cd ${WORKDIR}
rm boost_1_63_0.tar.gz*
if [ ! -d boost_1_63_0 ]; then
  wget https://netcologne.dl.sourceforge.net/project/boost/boost/1.63.0/boost_1_63_0.tar.gz
  tar -xzf boost_1_63_0.tar.gz
  cd boost_1_63_0
  ./bootstrap.sh --with-libraries=date_time,filesystem,program_options,regex,system,test,thread >/dev/null
  ./b2 >/dev/null
else
  cd boost_1_63_0
fi
sudo ./b2 install >/dev/null

cd ${WORKDIR}
rm Markdown-2.6.8.tar.gz*
if [ ! -d Markdown-2.6.8 ]; then
  wget https://pypi.python.org/packages/1d/25/3f6d2cb31ec42ca5bd3bfbea99b63892b735d76e26f20dd2dcc34ffe4f0d/Markdown-2.6.8.tar.gz
  tar -xzf Markdown-2.6.8.tar.gz
  cd Markdown-2.6.8
  ./setup.py build >/dev/null
else
  cd Markdown-2.6.8
fi
sudo ./setup.py install

cd ${WORKDIR}
rm Cheetah-2.4.4.tar.gz*
if [ ! -d Cheetah-2.4.4 ]; then
  wget https://pypi.python.org/packages/cd/b0/c2d700252fc251e91c08639ff41a8a5203b627f4e0a2ae18a6b662ab32ea/Cheetah-2.4.4.tar.gz
  tar -xzf Cheetah-2.4.4.tar.gz
  cd Cheetah-2.4.4
  ./setup.py build >/dev/null
else
  cd Cheetah-2.4.4
fi
sudo ./setup.py install

cd ${WORKDIR}
rm volk-1.3.tar.gz*
if [ ! -d volk-1.3 ]; then
  wget http://libvolk.org/releases/volk-1.3.tar.gz
  tar -xzf volk-1.3.tar.gz
  cd volk-1.3
  mkdir build && cd build
  cmake ..
  make
else
  cd volk-1.3/build
fi
sudo make install

cd ${WORKDIR}
if [ ! -d gnuradio ]; then
  git clone https://github.com/gnuradio/gnuradio
  cd gnuradio
  mkdir build
  cd build
else
  cd gnuradio
  git pull
  cd build
fi
cmake -DENABLE_INTERNAL_VOLK:BOOL=OFF -DENABLE_GR_FEC:BOOL=OFF -DENABLE_GR_DIGITAL:BOOL=OFF -DENABLE_GR_DTV:BOOL=OFF -DENABLE_GR_ATSC:BOOL=OFF -DENABLE_GR_AUDIO:BOOL=OFF -DENABLE_GR_CHANNELS:BOOL=OFF -DENABLE_GR_NOAA:BOOL=OFF -DENABLE_GR_PAGER:BOOL=OFF -DENABLE_GR_TRELLIS:BOOL=OFF -DENABLE_GR_VOCODER:BOOL=OFF ..
make
sudo make install >/dev/null

cd ${WORKDIR}
if [ ! -d libsigrok ]; then
  git clone https://github.com/sschnelle/libsigrok/
  cd libsigrok
  ./autogen.sh
  ./configure --disable-all-drivers --enable-bindings --enable-cxx
  make
else
  cd libsigrok
fi
sudo make install

cd ${WORKDIR}
rm libsigrokdecode-0.4.1.tar.gz*
if [ ! -d libsigrokdecode-0.4.1 ]; then
  wget http://sigrok.org/download/source/libsigrokdecode/libsigrokdecode-0.4.1.tar.gz
  tar -xzvf libsigrokdecode-0.4.1.tar.gz
  cd libsigrokdecode-0.4.1
  ./configure
  make
else
  cd libsigrokdecode-0.4.1
fi
sudo make install

cd  ${WORKDIR}
if [ ! -d qwt ]; then
  git clone https://github.com/osakared/qwt.git -b qwt-6.1-multiaxes
  cd qwt
  curl https://raw.githubusercontent.com/analogdevicesinc/scopy/osx/qwt-6.1-multiaxes.patch |patch -p1 --forward
  /opt/qt59/bin/qmake qwt.pro
  make
else
  cd qwt
fi
sudo make install

cd ${WORKDIR}
rm qwtpolar-1.1.1.tar.bz2*
if [ ! -d qwtpolar-1.1.1 ]; then
  wget https://downloads.sourceforge.net/project/qwtpolar/qwtpolar/1.1.1/qwtpolar-1.1.1.tar.bz2
  tar xvjf qwtpolar-1.1.1.tar.bz2
  cd qwtpolar-1.1.1
  curl -o qwtpolar-qwt-6.1-compat.patch https://raw.githubusercontent.com/analogdevicesinc/scopy-flatpak/master/qwtpolar-qwt-6.1-compat.patch
  patch -p1 < qwtpolar-qwt-6.1-compat.patch

  # Disable components that we won't build
  sed -i "/^QWT_POLAR_CONFIG\\s*+=\\s*QwtPolarExamples$/s/^/#/g" qwtpolarconfig.pri
  sed -i "/^QWT_POLAR_CONFIG\\s*+=\\s*QwtPolarDesigner$/s/^/#/g" qwtpolarconfig.pri
  # Fix prefix
  sed -i "s/^\\s*QWT_POLAR_INSTALL_PREFIX.*$/QWT_POLAR_INSTALL_PREFIX=\/usr\/local/g" qwtpolarconfig.pri
  sed -i "/^QWT_POLAR_INSTALL_HEADERS/s/$/\/qwt/g" qwtpolarconfig.pri
  cat qwtpolarconfig.pri | grep QWT_POLAR_INSTALL_PREFIX
  /opt/qt59/bin/qmake LIBS+="-L/usr/local/lib -lqwt" INCLUDEPATH+="/usr/local/include/qwt" qwtpolar.pro
  make
else
  cd qwtpolar-1.1.1
fi
sudo make install

cd ${WORKDIR}
if [ ! -d libiio ]; then
  git clone https://github.com/analogdevicesinc/libiio
  cd libiio && mkdir build
  cd build
  cmake -DCMAKE_INSTALL_LIBDIR:STRING=lib -DINSTALL_UDEV_RULE:BOOL=OFF -DPYTHON_BINDINGS:BOOL=OFF -DCSHARP_BINDINGS:BOOL=OFF -DWITH_TESTS:BOOL=OFF -DWITH_DOC:BOOL=OFF -DWITH_IIOD:BOOL=OFF -DWITH_LOCAL_BACKEND:BOOL=OFF -DWITH_MATLAB_BINDINGS_API:BOOL=OFF ..
  make
else
  cd libiio
  git pull
  cd build
fi
sudo make install

cd ${WORKDIR}
if [ ! -d libad9361-iio ]; then
  git clone https://github.com/analogdevicesinc/libad9361-iio
  cd libad9361-iio
  mkdir build
  cd build
  cmake ..
  make
else
  cd libad9361-iio
  git pull
  cd build
fi
sudo make install

cd ${WORKDIR}
if [ ! -d gr-iio ]; then
  git clone https://github.com/analogdevicesinc/gr-iio
  cd gr-iio
  mkdir build
  cd build
  cmake ..
  make
else
  cd gr-iio
  git pull
  cd build
fi
sudo make install
