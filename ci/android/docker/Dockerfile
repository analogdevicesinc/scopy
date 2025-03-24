FROM ubuntu:20.04

ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Bucharest

# Set timezone and install dependencies
RUN set -ex && \
    ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && \
    echo $TZ > /etc/timezone && \
    apt-get update -y && \
    apt-get upgrade -y && \
    apt-get install -y \
        autoconf-archive \
        autogen \
        bison \
        build-essential \
        cmake \
        flex \
        gettext \
        git \
        libglib2.0-dev \
        libncurses5 \
        libqt5gui5 \
        libtool \
        libxcb-xinerama0 \
        libxkbcommon-x11-0 \
        mm-common\
        ninja-build \
        openjdk-17-jre \
        pip \
        pkg-config \
        python-dev \
        python-mako \
        python-six \
        python3-mako \
        python3-numpy \
        subversion \
        sudo \
        swig3.0 \
        texinfo \
        texinfo \
        unzip \
        vim \
        wget \
        xserver-xorg;

# Create user and group
RUN set -ex && \
    groupadd -g 1000 -r $USER && \
    useradd -u 1000 -g 1000 --create-home -r $USER && \
    # Change password
    echo "$USER:$USER" | chpasswd && \
    # Make sudo passwordless
    echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
    usermod -aG sudo $USER && \
    usermod -aG plugdev $USER

USER $USER


RUN --mount=type=secret,id=qt_user,uid=1000,target=/run/secrets/qt_user \
    --mount=type=secret,id=qt_password,uid=1000,target=/run/secrets/qt_password \
    set -ex && \ 
    wget https://download.qt.io/official_releases/online_installers/qt-unified-linux-x64-online.run && \
    chmod +x qt-unified-linux-x64-online.run && \
    ./qt-unified-linux-x64-online.run install \
        --ao --al --aa DownloadError=Retry -c \
        -m $(cat /run/secrets/qt_user) \
        --pw $(cat /run/secrets/qt_password) \
        qt.qt5.5152.android qt.tools.cmake && \
    rm qt-unified-linux-x64-online.run && \
    sudo apt remove -y qtchooser && \
    # Symlink Qt Creator binary
    sudo ln -s /home/$USER/Qt/Tools/QtCreator/bin/qtcreator.sh /usr/local/bin/qtcreator

# Clone and build Scopy Android dependencies
WORKDIR /home/$USER/src
ARG REPO_LINK=https://github.com/analogdevicesinc/scopy-android-deps.git
ARG REPO_BRANCH=Scopy-1.5.0
RUN git clone "${REPO_LINK}" --branch "${REPO_BRANCH}" --recursive

WORKDIR /home/$USER/src/scopy-android-deps
RUN ./build_androiddeployqt.sh

ENV ANDROID_TOOLCHAIN_LOCATION=/home/$USER/src/scopy-android-deps
RUN ./android_deps_build.sh aarch64 $USER