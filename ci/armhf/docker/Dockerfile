FROM --platform=linux/amd64 ubuntu:20.04 AS start
SHELL ["/bin/bash", "-c"]
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Europe/Bucharest
ENV CI_SCRIPT=ON
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && \
	apt-get -y upgrade && \
	apt-get install -y apt-utils sudo git wget flex bison pkg-config make python3 pip vim python-is-python3
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



FROM start AS sysroot_builder
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
#Copy all scopy/ci/armhf folder inside the image
COPY * /home/${USER}
RUN ./create_sysroot.sh \
	install_packages \
	move_and_extract_sysroot \
	fix_relativelinks
RUN ./build_qt.sh \
	install_packages \
	download_qt \
	download_crosscompiler \
	build_qt5.15.2
RUN ./armhf_build_process.sh \
	install_packages \
	download_cmake \
	download_crosscompiler \
	clone \
	build_deps



FROM start
ARG USER=runner
ENV DEBIAN_FRONTEND=noninteractive
ARG STAGING=/home/${USER}/scopy/ci/armhf/staging
COPY --from=sysroot_builder /home/${USER}/armhf_build_process.sh /home/${USER}
COPY --from=sysroot_builder /home/${USER}/armhf_build_config.sh /home/${USER}
COPY --from=sysroot_builder /home/${USER}/build-status /home/${USER}/build-status
RUN mkdir -p /home/${USER}/sysroot
COPY --from=sysroot_builder ${STAGING}/sysroot/usr /home/${USER}/sysroot/usr
COPY --from=sysroot_builder ${STAGING}/sysroot/share /home/${USER}/sysroot/share
COPY --from=sysroot_builder ${STAGING}/sysroot/include /home/${USER}/sysroot/include
WORKDIR /home/${USER}/sysroot
RUN ln -s usr/bin bin && ln -s usr/lib lib && ln -s usr/sbin sbin
WORKDIR /home/${USER}
RUN /home/${USER}/armhf_build_process.sh install_packages download_cmake download_crosscompiler  && \
	mv ${STAGING} /home/${USER} && \
	rm -rf /home/${USER}/armhf_build_process.sh /home/${USER}/armhf_build_config.sh /home/${USER}/scopy
