# Scopy Android (Qt6) build environment - arm64-v8a
# =================================================
# Build context is ci/android, so the two ci scripts can be COPYed in:
#   cd ci/android && TAG=testing ./create_docker_image.sh android_qt6
# Published as docker.cloudsmith.io/adi/scopy-dockers/scopy2-android-qt6 by
# ci/android/create_docker_image.sh and .github/workflows/push-docker.yml.
#
# Everything slow is baked in: JDK 17, the Android SDK platform/build-tools/NDK, Qt 6.8.3 for
# android_arm64_v8a plus the matching host Qt, and the nine cross-compiled dependencies. CI then
# runs only "android_deps_build.sh build_scopy".
#
# Deliberately NOT squashed with the "FROM scratch / COPY --from=0 / /" trick the other Scopy
# Dockerfiles end with: that copies the filesystem but discards ENV/USER/WORKDIR, and this image's
# ENV is load-bearing - STAGING_AREA is the only thing that tells android_toolchain.sh where the
# baked dependencies are, and losing it yields a green build and a plugin-less APK. Space is
# reclaimed by pruning inside the RUN that creates the data instead.

ARG BASE_IMAGE=ubuntu:24.04

FROM ${BASE_IMAGE}
SHELL ["/bin/bash", "-c"]

ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive

# No "ENV CI_SCRIPT=ON" here, unlike ci/ubuntu/docker_ubuntu/Dockerfile: android_toolchain.sh does
# "[ "$CI_SCRIPT" == "ON" ] && SRC_DIR=$GITHUB_WORKSPACE", so baking it in would blank SRC_DIR for
# anyone running this container interactively. The workflow passes -e CI_SCRIPT=ON explicitly.

ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && \
	apt-get -y upgrade && \
	apt-get install -y apt-utils sudo tzdata software-properties-common

# uid/gid 1001 is the "runner" user on GitHub-hosted runners. Matching it means the container
# writes into the bind-mounted $GITHUB_WORKSPACE as its real owner - no chown step, and the APK
# comes back readable by actions/upload-artifact.
RUN groupadd -g 1001 -r $USER && \
	useradd -u 1001 -g 1001 --create-home -r $USER

#Change password
RUN echo "$USER:$USER" | chpasswd

#Make sudo passwordless
RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
	usermod -aG sudo $USER

# autoconf/automake/libtool: libserialport's autogen.sh runs "autoreconf --force --install".
# ninja-build: the verified APK build used the Ninja generator.
# openjdk-17-jdk: Qt 6.8.3 ships Android Gradle Plugin 8.6.0, which requires Java 17.
# libglib2.0-dev: the HOST Qt tools link libglib-2.0.so.0 and libgthread-2.0.so.0 (ldd
#   androiddeployqt). The runtime package was renamed libglib2.0-0t64 on 24.04, so the -dev name
#   is used because it is stable across releases.
# No CMake download: 24.04 ships 3.28.3, and the highest cmake_minimum_required in the dependency
# set is qcoro's 3.18.4 (Qt 6.8 Android wants >= 3.21). A newer CMake would be a liability - 4.x
# rejects cmake_minimum_required(<3.5) and libiio v0.26 declares 3.10.
RUN apt-get install -y --no-install-recommends \
		autoconf automake libtool \
		build-essential cmake ninja-build make pkg-config \
		ca-certificates git wget unzip file \
		libglib2.0-dev \
		openjdk-17-jdk \
		python3-pip && \
	rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# aqtinstall, the mechanism ci/ubuntu/ubuntu_build_process.sh already uses, with the Qt version
# pinned repo-wide at 6.8.3.
#
# Two explicit installs rather than --autodesktop: the host line is then exactly what the desktop
# image runs, and BOTH sides get Qt3D, which packages/imu/plugins/imuanalyzer find_package()s as
# REQUIRED (--autodesktop would install the desktop base only). Desktop first.
#
# "all_os" is not a typo: Qt moved the Android and WASM targets out of the per-host trees into
# all_os at 6.7.0, which is why "aqt list-qt linux android" appears to cap at 6.7.3.
#
# qtsvg/qtdeclarative/qttools are NOT -m add-on modules in Qt6 aqt - they ship inside the base
# install-qt package, and passing them to -m fails.
RUN pip3 install --no-cache-dir --break-system-packages aqtinstall && \
	python3 -m aqt install-qt -O /opt/Qt linux  desktop 6.8.3 linux_gcc_64      -m qt3d && \
	python3 -m aqt install-qt -O /opt/Qt all_os android 6.8.3 android_arm64_v8a -m qt3d && \
	rm -rf /root/.cache/pip

# The Android kit's bin/qmake6 is a /bin/sh wrapper with the HOST qmake's absolute path baked in,
# and bin/target_qt.conf points back at ../../gcc_64; aqt rewrites both at install time. build_qwt
# runs through that wrapper, so assert it here rather than discovering it 30 minutes into a
# dependency build. Same for Qt3D, whose absence would otherwise only surface at Scopy configure.
RUN test -x /opt/Qt/6.8.3/gcc_64/bin/qmake6 && \
	grep -q '/opt/Qt/6.8.3/gcc_64/bin/qmake6' /opt/Qt/6.8.3/android_arm64_v8a/bin/qmake6 && \
	test -d /opt/Qt/6.8.3/android_arm64_v8a/lib/cmake/Qt63DCore && \
	test -x /opt/Qt/6.8.3/gcc_64/bin/androiddeployqt

# From here on the image is uid-1001-only, by design and by necessity:
#   - /home/runner is mode 750, so another uid cannot even traverse to the SDK, the NDK or the
#     baked dependencies under $STAGING_AREA;
#   - Scopy's configure step WRITES into the source tree (cmake/Modules/ScopyWhatsNew.cmake,
#     ScopyAbout.cmake's configure_file into resources/, and ScopyStyle.cmake's style_generator.py
#     emitting gui/include/gui/style_properties.h), so the container user must own the checkout.
# On a GitHub runner both hold: $GITHUB_WORKSPACE is uid 1001. Driving this image by hand against a
# checkout owned by some other uid needs that uid plus runner's group, and a writable HOME:
#   docker run --user $(id -u):$(id -g) --group-add 1001 -e HOME=/tmp/h -v /tmp/h:/tmp/h ...
USER $USER

# One ABI per image, per Qt kit - CMakeLists.txt deliberately leaves QT_ANDROID_ABIS unset because
# multi-ABI sub-builds do not forward -D cache variables.
ENV ABI=arm64-v8a
# android_toolchain.sh derives ANDROID_NDK_ROOT from this, and qt.toolchain.cmake reads
# $ENV{ANDROID_NDK_ROOT} to chainload the NDK's own android.toolchain.cmake.
ENV ANDROID_SDK_ROOT=/home/${USER}/Android/Sdk
# The dependencies must NOT live under the checked-out repo: in CI a fresh clone is bind-mounted
# over $GITHUB_WORKSPACE, which would hide them. android_toolchain.sh honours $STAGING_AREA, so
# point it into the image; deps land in $STAGING_AREA/$ABI/dependencies.
ENV STAGING_AREA=/home/${USER}/android-staging

WORKDIR /home/${USER}/scripts
COPY --chown=1001:1001 android_toolchain.sh android_deps_build.sh ./

# Two RUNs on purpose: the SDK+NDK layer is ~2.5 GB, and a dependency-pin change must not
# re-download it while iterating locally. download_sdk is idempotent, so configure_system
# re-enters it for free.
RUN ./android_deps_build.sh download_sdk

# The dependency sources are ~3 GB of git clones; pruned in the SAME RUN so they never enter a
# layer. build_deps ends in verify_deps, which fails this RUN - and so the whole docker build - on
# a wrong-architecture or versioned-SONAME library.
RUN ./android_deps_build.sh configure_system && \
	rm -rf ${STAGING_AREA}/src \
		${ANDROID_SDK_ROOT}/.temp \
		${ANDROID_SDK_ROOT}/.downloadIntermediates

WORKDIR /home/${USER}

# Clean image. The scripts are deleted on purpose: CI runs them from the MOUNTED checkout, so a
# job always builds whatever the branch says - same model as ci/ubuntu.
RUN rm -rf /home/${USER}/scripts
