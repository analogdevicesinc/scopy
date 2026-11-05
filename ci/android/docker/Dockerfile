# Scopy Android (Qt6) build environment - arm64-v8a
# Build context is ci/android:  cd ci/android && ./create_docker_image.sh android_qt6
# Published as docker.cloudsmith.io/adi/scopy-dockers/scopy2-android-qt6.
# Carries the SDK/NDK/JDK, Qt for Android plus the host Qt, and the cross-compiled dependencies.

ARG BASE_IMAGE=ubuntu:24.04

FROM ${BASE_IMAGE}
SHELL ["/bin/bash", "-c"]

ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive

ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && \
	apt-get -y upgrade && \
	apt-get install -y apt-utils sudo tzdata software-properties-common

# uid/gid 1001 matches the GitHub runner user, so the bind-mounted workspace stays writable.
RUN groupadd -g 1001 -r $USER && \
	useradd -u 1001 -g 1001 --create-home -r $USER

#Change password
RUN echo "$USER:$USER" | chpasswd

#Make sudo passwordless
RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
	usermod -aG sudo $USER

# autotools for libserialport, JDK 17 for Gradle, libglib2.0-dev for the host Qt tools.
# 24.04's cmake 3.28.3 is new enough, so no cmake download.
RUN apt-get install -y --no-install-recommends \
		autoconf automake libtool \
		build-essential cmake ninja-build make pkg-config \
		ca-certificates git wget unzip file \
		libglib2.0-dev \
		openjdk-17-jdk \
		python3-pip && \
	rm -rf /var/lib/apt/lists/*

ENV JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

# Android and WASM live in aqt's "all_os" host tree since Qt 6.7. Both installs need -m qt3d for
# the imuanalyzer plugin; desktop first, since the Android kit's qmake wrapper points at it.
RUN pip3 install --no-cache-dir --break-system-packages aqtinstall && \
	python3 -m aqt install-qt -O /opt/Qt linux  desktop 6.8.3 linux_gcc_64      -m qt3d && \
	python3 -m aqt install-qt -O /opt/Qt all_os android 6.8.3 android_arm64_v8a -m qt3d && \
	rm -rf /root/.cache/pip

# Fail here rather than mid dependency build if aqt did not patch the qmake wrapper.
RUN test -x /opt/Qt/6.8.3/gcc_64/bin/qmake6 && \
	grep -q '/opt/Qt/6.8.3/gcc_64/bin/qmake6' /opt/Qt/6.8.3/android_arm64_v8a/bin/qmake6 && \
	test -d /opt/Qt/6.8.3/android_arm64_v8a/lib/cmake/Qt63DCore && \
	test -x /opt/Qt/6.8.3/gcc_64/bin/androiddeployqt

USER $USER

# One ABI per image, per Qt kit.
ENV ABI=arm64-v8a
ENV ANDROID_SDK_ROOT=/home/${USER}/Android/Sdk
# Outside the repo: CI bind-mounts a fresh clone over the workspace.
ENV STAGING_AREA=/home/${USER}/android-staging

WORKDIR /home/${USER}/scripts
COPY --chown=1001:1001 android_toolchain.sh android_deps_build.sh ./

# Separate layer so a dependency change does not re-download the NDK.
RUN ./android_deps_build.sh download_sdk

# Sources pruned in the same layer. verify_deps fails the build on a bad dependency.
RUN ./android_deps_build.sh configure_system && \
	rm -rf ${STAGING_AREA}/src \
		${ANDROID_SDK_ROOT}/.temp \
		${ANDROID_SDK_ROOT}/.downloadIntermediates

WORKDIR /home/${USER}

# CI runs the scripts from the mounted checkout, not these copies.
RUN rm -rf /home/${USER}/scripts
