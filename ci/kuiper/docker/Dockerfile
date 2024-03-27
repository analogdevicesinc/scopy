# docker build -t cristianbindea/scopy-kuiper

### innitial configuration stage
FROM --platform=linux/amd64 ubuntu:20.04 AS start
SHELL ["/bin/bash", "-c"]

ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive

ENV SYSROOT_LOCATION=/home/${USER}/scopy-build/sysroot

ENV TZ=Europe/Bucharest
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && \
	apt-get -y upgrade && \
	apt-get install -y apt-utils sudo git wget flex bison pkg-config make python3 pip vim

RUN groupadd -g 1000 -r $USER && \
	useradd -u 1000 -g 1000 --create-home -r $USER

#Change password
RUN echo "$USER:$USER" | chpasswd

#Make sudo passwordless
RUN echo "${USER} ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/90-$USER && \
	usermod -aG sudo $USER && \
	usermod -aG plugdev $USER

USER $USER
WORKDIR /home/${USER}

### sysroot creation stage
FROM start AS sysroot_builder

#RUN git clone https://github.com/analogdevicesinc/scopy --branch kuiper-worflow scopy
# combina comenzile de RUN
RUN scopy/ci/kuiper/create_sysroot.sh install_packages download_kuiper install_qemu extract_sysroot
RUN scopy/ci/kuiper/create_sysroot.sh configure_sysroot

### Qt build stage
FROM start AS qt_builder

COPY --from=sysroot_builder ${SYSROOT_LOCATION} ${SYSROOT_LOCATION}
WORKDIR /home/${USER}
#RUN git clone https://github.com/analogdevicesinc/scopy --branch kuiper-worflow scopy
RUN scopy/ci/kuiper/build_qt.sh install_packages download_qt download_crosscompiler build_qt

### Scopy deps stage
FROM start AS scopy_builder

COPY --from=qt_builder ${SYSROOT_LOCATION} ${SYSROOT_LOCATION}
#RUN git clone https://github.com/analogdevicesinc/scopy --branch kuiper-worflow scopy
RUN scopy/ci/kuiper/kuiper_build_process.sh install_packages download_cmake download_crosscompiler clone build_deps